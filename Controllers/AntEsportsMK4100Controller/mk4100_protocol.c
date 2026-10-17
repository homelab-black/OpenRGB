#define _POSIX_C_SOURCE 200809L
#include "mk4100_protocol.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

#define DEFAULT_TIMEOUT_MS 500u

enum match_kind {
    MATCH_CONFIG_READ,
    MATCH_EFFECT_CONFIG_READ,
    MATCH_CONFIG_WRITE,
    MATCH_RGB_READ,
    MATCH_RGB_WRITE,
    MATCH_RGB_SINGLE_WRITE
};

typedef struct match_spec {
    enum match_kind kind;
    unsigned offset;
    const uint8_t *payload;
    size_t length;
} match_spec;

static double monotonic_ms(void)
{
#if defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter)) return 0.0;
    return (double)counter.QuadPart * 1000.0 / (double)frequency.QuadPart;
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
#endif
}

static int fail(mk4100_device *d, const char *fmt, ...)
{
    if (d) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(d->error, sizeof(d->error), fmt, args);
        va_end(args);
    }
    return -1;
}

static int matches_response(const uint8_t *r, const match_spec *m)
{
    if (r[0] != 0xAAu) return 0;
    switch (m->kind) {
    case MATCH_CONFIG_READ:
        /* The keyboard has been observed returning both AA 0A 0A and AA 0A 0B
           for this read. Accept either header only when the payload at offset 5
           looks like the known 11-byte config (mode 0..16, brightness 0..4). */
        return r[1] == 0x0Au &&
               (r[2] == 0x0Au || r[2] == MK4100_CONFIG_BYTES) &&
               r[7] <= MK4100_CUSTOM_MODE && r[8] <= 4u;
    case MATCH_EFFECT_CONFIG_READ:
        return r[1] == 0x16u;
    case MATCH_CONFIG_WRITE:
        return r[1] == 0x0Bu && r[2] == 0x01u;
    case MATCH_RGB_READ:
        return r[1] == 0x13u && r[2] == 0x3Au &&
               r[3] == (uint8_t)(m->offset & 0xFFu) &&
               r[4] == (uint8_t)((m->offset >> 8) & 0xFFu);
    case MATCH_RGB_WRITE:
        return r[1] == 0x12u && r[2] == 0x01u &&
               r[3] == (uint8_t)(m->offset & 0xFFu) &&
               r[4] == (uint8_t)((m->offset >> 8) & 0xFFu) &&
               r[5] == 0u && r[6] == 0u && r[7] == 0u &&
               memcmp(r + 8, m->payload, m->length) == 0;
    case MATCH_RGB_SINGLE_WRITE:
        return r[1] == 0x14u;
    }
    return 0;
}

static int transact(mk4100_device *d,
                    const uint8_t out[MK4100_REPORT_BYTES],
                    const match_spec *spec, const char *label,
                    uint8_t reply[MK4100_REPORT_BYTES])
{
    if (!d || !d->transport.write_report || !d->transport.read_report)
        return fail(d, "Transport is not initialized for %s", label);
    if (d->transport.write_report(d->transport.context, out) != 0)
        return fail(d, "HID write failed for %s", label);

    const double deadline = monotonic_ms() + (double)d->timeout_ms;
    for (;;) {
        const double remaining = deadline - monotonic_ms();
        if (remaining <= 0.0)
            return fail(d, "Timed out waiting for matching acknowledgement: %s", label);
        unsigned wait_ms = (unsigned)remaining;
        if (wait_ms == 0u) wait_ms = 1u;
        uint8_t in[MK4100_REPORT_BYTES];
        const int result = d->transport.read_report(d->transport.context, in, wait_ms);
        if (result < 0) return fail(d, "HID read failed while waiting for %s", label);
        if (result == 0) continue;
        if (matches_response(in, spec)) {
            if (reply) memcpy(reply, in, MK4100_REPORT_BYTES);
            return 0;
        }
    }
}

static size_t chunk_length(unsigned offset)
{
    const size_t remaining = MK4100_RGB_BYTES - (size_t)offset;
    return remaining < MK4100_RGB_CHUNK_BYTES ? remaining : MK4100_RGB_CHUNK_BYTES;
}

static int read_config_internal(mk4100_device *d,
                                uint8_t out_config[MK4100_CONFIG_BYTES])
{
    uint8_t out[MK4100_REPORT_BYTES] = {0};
    uint8_t in[MK4100_REPORT_BYTES];
    out[0] = 0x06u; out[1] = 0x0Au;
    const match_spec spec = { MATCH_CONFIG_READ, 0u, NULL, 0u };
    if (transact(d, out, &spec, "read lighting configuration", in) != 0) return -1;
    memcpy(out_config, in + 5, MK4100_CONFIG_BYTES);
    return 0;
}

static int write_config_internal(mk4100_device *d,
                                 const uint8_t config[MK4100_CONFIG_BYTES])
{
    uint8_t out[MK4100_REPORT_BYTES] = {0};
    out[0] = 0x06u; out[1] = 0x0Bu; out[2] = MK4100_CONFIG_BYTES;
    memcpy(out + 5, config, MK4100_CONFIG_BYTES);
    const match_spec spec = { MATCH_CONFIG_WRITE, 0u, NULL, 0u };
    if (transact(d, out, &spec, "write lighting configuration", NULL) != 0) {
        d->config_trusted = 0;
        return -1;
    }
    memcpy(d->config, config, MK4100_CONFIG_BYTES);
    d->config_trusted = 1;
    return 0;
}

static int read_rgb_chunk(mk4100_device *d, unsigned offset,
                          uint8_t *dst, size_t length)
{
    uint8_t out[MK4100_REPORT_BYTES] = {0};
    uint8_t in[MK4100_REPORT_BYTES];
    out[0] = 0x06u; out[1] = 0x13u; out[2] = 0x3Au;
    out[3] = (uint8_t)(offset & 0xFFu);
    out[4] = (uint8_t)((offset >> 8) & 0xFFu);
    const match_spec spec = { MATCH_RGB_READ, offset, NULL, length };
    if (transact(d, out, &spec, "read RGB buffer chunk", in) != 0) return -1;
    memcpy(dst, in + 8, length);
    return 0;
}

static int read_rgb_internal(mk4100_device *d, uint8_t rgb[MK4100_RGB_BYTES])
{
    for (unsigned offset = 0u; offset < MK4100_RGB_BYTES;
         offset += MK4100_RGB_CHUNK_BYTES) {
        const size_t len = chunk_length(offset);
        if (read_rgb_chunk(d, offset, rgb + offset, len) != 0) return -1;
    }
    return 0;
}

static int write_rgb_chunk(mk4100_device *d, unsigned offset,
                           const uint8_t *payload, size_t length)
{
    if (offset >= MK4100_RGB_BYTES || offset % MK4100_RGB_CHUNK_BYTES != 0u ||
        length != chunk_length(offset))
        return fail(d, "Invalid vendor RGB chunk: offset=%u length=%zu", offset, length);

    uint8_t out[MK4100_REPORT_BYTES] = {0};
    out[0] = 0x06u; out[1] = 0x12u; out[2] = (uint8_t)(length + 3u);
    out[3] = (uint8_t)(offset & 0xFFu);
    out[4] = (uint8_t)((offset >> 8) & 0xFFu);
    memcpy(out + 8, payload, length);
    const match_spec spec = { MATCH_RGB_WRITE, offset, payload, length };
    char label[80];
    (void)snprintf(label, sizeof(label), "RGB write at offset %u", offset);
    if (transact(d, out, &spec, label, NULL) != 0) {
        d->rgb_trusted = 0;
        return -1;
    }
    memcpy(d->rgb + offset, payload, length);
    return 0;
}

/* Live frame writes use 54 RGB bytes (18 whole keys). The report still has
   room for 56 bytes after its protocol header, so duplicate the first two bytes
   of the next target segment in that overlap. Live chunks are sent descending,
   meaning these overlap bytes repeat values already written by the higher chunk
   if the firmware copies all 56 bytes instead of respecting the declared length. */
#define MK4100_RGB_LIVE_CHUNK_BYTES 54u

static int write_rgb_live_chunk(mk4100_device *d, unsigned offset,
                                const uint8_t *payload, size_t length,
                                const uint8_t *whole_frame)
{
    if (offset >= MK4100_RGB_BYTES || offset % MK4100_RGB_LIVE_CHUNK_BYTES != 0u ||
        offset % 3u != 0u ||
        length != ((MK4100_RGB_BYTES - (size_t)offset < MK4100_RGB_LIVE_CHUNK_BYTES)
                   ? MK4100_RGB_BYTES - (size_t)offset : MK4100_RGB_LIVE_CHUNK_BYTES) ||
        length % 3u != 0u || !whole_frame)
        return fail(d, "Invalid live RGB chunk: offset=%u length=%zu", offset, length);

    uint8_t out[MK4100_REPORT_BYTES] = {0};
    out[0] = 0x06u; out[1] = 0x12u; out[2] = (uint8_t)(length + 3u);
    out[3] = (uint8_t)(offset & 0xFFu);
    out[4] = (uint8_t)((offset >> 8) & 0xFFu);
    memcpy(out + 8u, payload, length);

    if(length == MK4100_RGB_LIVE_CHUNK_BYTES &&
       (size_t)offset + length + 2u <= MK4100_RGB_BYTES)
    {
        out[8u + length] = whole_frame[offset + length];
        out[8u + length + 1u] = whole_frame[offset + length + 1u];
    }

    const match_spec spec = { MATCH_RGB_WRITE, offset, payload, length };
    char label[80];
    (void)snprintf(label, sizeof(label), "live RGB write at offset %u", offset);
    if(transact(d, out, &spec, label, NULL) != 0)
    {
        d->rgb_trusted = 0;
        return -1;
    }
    memcpy(d->rgb + offset, payload, length);
    return 0;
}

static int ensure_config(mk4100_device *d)
{
    if (d->config_trusted) return 0;
    uint8_t config[MK4100_CONFIG_BYTES];
    if (read_config_internal(d, config) != 0) return -1;
    memcpy(d->config, config, sizeof(config));
    d->config_trusted = 1;
    return 0;
}

int mk4100_init(mk4100_device *d, const mk4100_transport *transport,
                unsigned timeout_ms)
{
    if (!d || !transport || !transport->write_report || !transport->read_report)
        return -1;
    memset(d, 0, sizeof(*d));
    d->transport = *transport;
    d->timeout_ms = timeout_ms ? timeout_ms : DEFAULT_TIMEOUT_MS;

    if (read_config_internal(d, d->config) != 0) return -1;
    memcpy(d->original_config, d->config, MK4100_CONFIG_BYTES);
    d->config_trusted = 1;

    uint8_t first[MK4100_RGB_BYTES], second[MK4100_RGB_BYTES];
    if (read_rgb_internal(d, first) != 0 || read_rgb_internal(d, second) != 0) return -1;
    if (memcmp(first, second, sizeof(first)) != 0)
        return fail(d, "Two startup RGB snapshots differed; refusing untrusted state");

    memcpy(d->rgb, second, sizeof(second));
    memcpy(d->original_rgb, second, sizeof(second));
    d->rgb_trusted = 1;
    d->initialized = 1;
    d->error[0] = '\0';
    return 0;
}

void mk4100_deinit(mk4100_device *d)
{
    if (d) d->initialized = 0;
}

const char *mk4100_last_error(const mk4100_device *d)
{
    return (d && d->error[0]) ? d->error : "unspecified MK4100 protocol error";
}

int mk4100_read_config(mk4100_device *d,
                       uint8_t config_out[MK4100_CONFIG_BYTES])
{
    if (!d || !d->initialized || !config_out) return fail(d, "Device is not initialized");
    uint8_t config[MK4100_CONFIG_BYTES];
    if (read_config_internal(d, config) != 0) return -1;
    memcpy(d->config, config, sizeof(config)); d->config_trusted = 1;
    memcpy(config_out, config, sizeof(config));
    return 0;
}

int mk4100_read_effect_config_for_mode(mk4100_device *d, unsigned mode,
                                       uint8_t config_out[MK4100_CONFIG_BYTES])
{
    if (!d || !d->initialized || !config_out) return fail(d, "Device is not initialized");
    if (mode > 15u) return fail(d, "Firmware effect mode must be in range 0..15");

    /* Mirrors the vendor WebHID request: group 0x06, inner bytes
       [0x16, 0, 0, 0, 1, 0, mode]. The returned 11-byte record contains
       this effect's saved parameters; the following 0x0B write applies it. */
    uint8_t out[MK4100_REPORT_BYTES] = {0};
    uint8_t reply[MK4100_REPORT_BYTES];
    out[0] = 0x06u;
    out[1] = 0x16u;
    out[5] = 0x01u;
    out[7] = (uint8_t)mode;
    const match_spec spec = { MATCH_EFFECT_CONFIG_READ, mode, NULL, 0u };
    if (transact(d, out, &spec, "read mode-specific lighting configuration", reply) != 0) return -1;
    memcpy(config_out, reply + 5, MK4100_CONFIG_BYTES);
    config_out[2] = (uint8_t)mode;
    d->error[0] = '\0';
    return 0;
}

int mk4100_write_config(mk4100_device *d,
                        const uint8_t config[MK4100_CONFIG_BYTES])
{
    if (!d || !d->initialized || !config) return fail(d, "Device is not initialized");
    return write_config_internal(d, config);
}

int mk4100_select_custom_mode(mk4100_device *d)
{
    if (!d || !d->initialized) return fail(d, "Device is not initialized");
    if (ensure_config(d) != 0) return -1;
    if (d->config[2] == MK4100_CUSTOM_MODE) return 0;
    uint8_t config[MK4100_CONFIG_BYTES];
    memcpy(config, d->config, sizeof(config));
    config[2] = MK4100_CUSTOM_MODE;
    return write_config_internal(d, config);
}

int mk4100_set_brightness(mk4100_device *d, unsigned brightness)
{
    if (!d || !d->initialized) return fail(d, "Device is not initialized");
    if (brightness > 4u) return fail(d, "Brightness must be in range 0..4");
    if (ensure_config(d) != 0) return -1;
    uint8_t config[MK4100_CONFIG_BYTES];
    memcpy(config, d->config, sizeof(config));
    config[3] = (uint8_t)brightness;
    if (write_config_internal(d, config) != 0) return -1;
    uint8_t readback[MK4100_CONFIG_BYTES];
    if (read_config_internal(d, readback) != 0) return -1;
    memcpy(d->config, readback, sizeof(readback)); d->config_trusted = 1;
    if (d->config[3] != brightness)
        return fail(d, "Brightness readback mismatch: wanted %u, got %u",
                    brightness, (unsigned)d->config[3]);
    return 0;
}

int mk4100_read_rgb(mk4100_device *d, uint8_t rgb_out[MK4100_RGB_BYTES])
{
    if (!d || !d->initialized || !rgb_out) return fail(d, "Device is not initialized");
    return read_rgb_internal(d, rgb_out);
}

int mk4100_refresh_rgb(mk4100_device *d, int stable_double_read)
{
    if (!d || !d->initialized) return fail(d, "Device is not initialized");
    uint8_t rgb[MK4100_RGB_BYTES];
    if (read_rgb_internal(d, rgb) != 0) { d->rgb_trusted = 0; return -1; }
    if (stable_double_read) {
        uint8_t second[MK4100_RGB_BYTES];
        if (read_rgb_internal(d, second) != 0) { d->rgb_trusted = 0; return -1; }
        if (memcmp(rgb, second, sizeof(rgb)) != 0) {
            d->rgb_trusted = 0;
            return fail(d, "RGB readback was not stable across two snapshots");
        }
        memcpy(rgb, second, sizeof(rgb));
    }
    memcpy(d->rgb, rgb, sizeof(rgb)); d->rgb_trusted = 1;
    return 0;
}

int mk4100_set_key_rgb(mk4100_device *d, unsigned slot,
                       uint8_t red, uint8_t green, uint8_t blue)
{
    if (!d || !d->initialized) return fail(d, "Device is not initialized");
    if (slot >= MK4100_RGB_SLOTS) return fail(d, "RGB slot must be in range 0..125");
    if (!d->rgb_trusted && mk4100_refresh_rgb(d, 1) != 0) return -1;
    if (mk4100_select_custom_mode(d) != 0) return -1;

    const unsigned offset = slot * 3u;
    uint8_t out[MK4100_REPORT_BYTES] = {0};
    out[0] = 0x06u; out[1] = 0x14u; out[2] = 0x03u;
    out[3] = (uint8_t)(offset & 0xFFu);
    out[4] = (uint8_t)((offset >> 8) & 0xFFu);
    out[8] = red; out[9] = green; out[10] = blue;
    const uint8_t color[3] = {red, green, blue};
    const match_spec spec = { MATCH_RGB_SINGLE_WRITE, offset, color, 3u };
    char label[80];
    (void)snprintf(label, sizeof(label), "single-key RGB write at slot %u", slot);
    if (transact(d, out, &spec, label, NULL) != 0) { d->rgb_trusted = 0; return -1; }
    memcpy(d->rgb + offset, color, 3u);
    d->error[0] = '\0';
    return 0;
}

int mk4100_update_rgb_frame(mk4100_device *d,
                            const uint8_t rgb[MK4100_RGB_BYTES],
                            int verify_after_write,
                            mk4100_stats *stats)
{
    if (!d || !d->initialized || !rgb) return fail(d, "Device is not initialized or frame is null");
    if (stats) memset(stats, 0, sizeof(*stats));
    if (!d->rgb_trusted && mk4100_refresh_rgb(d, 1) != 0) return -1;
    if (ensure_config(d) != 0) return -1;
    if (d->config[2] != MK4100_CUSTOM_MODE)
        return fail(d, "Custom/UserLight mode 16 must be active for live RGB frame updates");

    /* These four mapped slots were observed to be corrupted when a live bulk
       report touches their 54-byte chunk boundary, even if their requested color
       is unchanged. Repair based on the reports actually sent, not on whether
       the edge key itself changed in the requested frame. */
    static const unsigned edge_slots[] = { 36u, 54u, 72u, 90u };
    int repair_edge[sizeof(edge_slots) / sizeof(edge_slots[0])] = { 0 };

    const double start = monotonic_ms();
    unsigned chunks = 0u, bytes = 0u;

    /* Descending 54-byte chunks align every payload boundary to a complete RGB
       triplet. The 2-byte report overlap repeats the next segment's target data. */
    for (unsigned offset = MK4100_RGB_BYTES; offset > 0u;)
    {
        offset -= MK4100_RGB_LIVE_CHUNK_BYTES;
        const size_t remaining = MK4100_RGB_BYTES - (size_t)offset;
        const size_t len = remaining < MK4100_RGB_LIVE_CHUNK_BYTES
                         ? remaining : MK4100_RGB_LIVE_CHUNK_BYTES;
        if (memcmp(d->rgb + offset, rgb + offset, len) == 0) continue;
        if (write_rgb_live_chunk(d, offset, rgb + offset, len, rgb) != 0) return -1;
        ++chunks;
        bytes += (unsigned)len;

        /* A live write may affect the 54 payload bytes plus the two overlap
           bytes supplied for firmware that copies the full 56-byte data area.
           A fragile edge slot must be repaired if any of its three RGB bytes
           was included in a report, even if that slot was not selected/changed. */
        size_t written_end = (size_t)offset + len;
        if (len == MK4100_RGB_LIVE_CHUNK_BYTES &&
            (size_t)offset + len + 2u <= MK4100_RGB_BYTES)
        {
            written_end += 2u;
        }
        for (size_t i = 0; i < sizeof(edge_slots) / sizeof(edge_slots[0]); ++i)
        {
            const size_t edge_begin = (size_t)edge_slots[i] * 3u;
            const size_t edge_end = edge_begin + 3u;
            if ((size_t)offset < edge_end && written_end > edge_begin)
            {
                repair_edge[i] = 1;
            }
        }
    }

    for (size_t i = 0; i < sizeof(edge_slots) / sizeof(edge_slots[0]); ++i)
    {
        if (!repair_edge[i]) continue;
        const size_t p = (size_t)edge_slots[i] * 3u;
        if (mk4100_set_key_rgb(d, edge_slots[i], rgb[p], rgb[p + 1u], rgb[p + 2u]) != 0)
        {
            char detail[sizeof(d->error)];
            (void)snprintf(detail, sizeof(detail), "%s", mk4100_last_error(d));
            return fail(d, "RGB boundary-key repair failed for slot %u: %s",
                        edge_slots[i], detail);
        }
    }

    if (verify_after_write)
    {
        uint8_t verify[MK4100_RGB_BYTES];
        if (read_rgb_internal(d, verify) != 0) { d->rgb_trusted = 0; return -1; }
        if (memcmp(verify, rgb, sizeof(verify)) != 0)
        {
            size_t i = 0u;
            while (i < sizeof(verify) && verify[i] == rgb[i]) ++i;
            d->rgb_trusted = 0;
            return fail(d, "RGB readback mismatch at byte %zu: wanted %u, read %u",
                        i, i < sizeof(verify) ? (unsigned)rgb[i] : 0u,
                        i < sizeof(verify) ? (unsigned)verify[i] : 0u);
        }
        memcpy(d->rgb, verify, sizeof(verify));
        d->rgb_trusted = 1;
    }

    if (stats)
    {
        stats->chunks_sent = chunks;
        stats->bytes_sent = bytes;
        stats->elapsed_ms = monotonic_ms() - start;
    }
    d->error[0] = '\0';
    return 0;
}

int mk4100_set_static_rgb_frame(mk4100_device *d,
                                const uint8_t rgb[MK4100_RGB_BYTES],
                                mk4100_stats *stats)
{
    if (!d || !d->initialized || !rgb) return fail(d, "Device is not initialized or frame is null");
    if (stats) memset(stats, 0, sizeof(*stats));
    if (ensure_config(d) != 0) return -1;

    uint8_t previous[MK4100_CONFIG_BYTES];
    memcpy(previous, d->config, sizeof(previous));
    uint8_t off[MK4100_CONFIG_BYTES];
    memcpy(off, previous, sizeof(off)); off[2] = 0u; off[6] = 0u;
    const double start = monotonic_ms();
    if (write_config_internal(d, off) != 0) return -1;

    d->rgb_trusted = 0;
    for (unsigned offset = 0u; offset < MK4100_RGB_BYTES;
         offset += MK4100_RGB_CHUNK_BYTES) {
        const size_t len = chunk_length(offset);
        if (write_rgb_chunk(d, offset, rgb + offset, len) != 0) return -1;
    }

    uint8_t verify[MK4100_RGB_BYTES];
    if (read_rgb_internal(d, verify) != 0) return -1;
    if (memcmp(verify, rgb, sizeof(verify)) != 0) {
        size_t i = 0u;
        while (i < sizeof(verify) && verify[i] == rgb[i]) ++i;
        return fail(d, "Static frame readback mismatch at byte %zu", i);
    }

    uint8_t custom[MK4100_CONFIG_BYTES];
    memcpy(custom, previous, sizeof(custom)); custom[2] = MK4100_CUSTOM_MODE;
    if (write_config_internal(d, custom) != 0) return -1;
    memcpy(d->rgb, verify, sizeof(verify)); d->rgb_trusted = 1;
    if (stats) {
        stats->chunks_sent = (MK4100_RGB_BYTES + MK4100_RGB_CHUNK_BYTES - 1u) / MK4100_RGB_CHUNK_BYTES;
        stats->bytes_sent = MK4100_RGB_BYTES;
        stats->elapsed_ms = monotonic_ms() - start;
    }
    d->error[0] = '\0';
    return 0;
}

int mk4100_restore(mk4100_device *d)
{
    if (!d || !d->initialized) return fail(d, "Device is not initialized");
    if (ensure_config(d) != 0) return -1;
    uint8_t off[MK4100_CONFIG_BYTES];
    memcpy(off, d->config, sizeof(off)); off[2] = 0u; off[6] = 0u;
    if (write_config_internal(d, off) != 0) return -1;

    d->rgb_trusted = 0;
    for (unsigned offset = 0u; offset < MK4100_RGB_BYTES;
         offset += MK4100_RGB_CHUNK_BYTES) {
        const size_t len = chunk_length(offset);
        if (write_rgb_chunk(d, offset, d->original_rgb + offset, len) != 0) return -1;
    }
    uint8_t verify[MK4100_RGB_BYTES];
    if (read_rgb_internal(d, verify) != 0 || memcmp(verify, d->original_rgb, sizeof(verify)) != 0)
        return fail(d, "Startup RGB buffer restore failed verification; lighting remains off");
    if (write_config_internal(d, d->original_config) != 0) return -1;
    memcpy(d->rgb, verify, sizeof(verify)); d->rgb_trusted = 1;
    memcpy(d->config, d->original_config, sizeof(d->config)); d->config_trusted = 1;
    d->error[0] = '\0';
    return 0;
}
