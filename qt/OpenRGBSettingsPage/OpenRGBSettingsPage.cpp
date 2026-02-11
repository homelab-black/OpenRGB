/*---------------------------------------------------------*\
| OpenRGBSettingsPage.cpp                                   |
|                                                           |
|   User interface for general settings page                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QUrl>
#include <QDesktopServices>
#include "AutoStart.h"
#include "OpenRGBSettingsPage.h"
#include "ui_OpenRGBSettingsPage.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "ProfileManager.h"

OpenRGBSettingsPage::OpenRGBSettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OpenRGBSettingsPage)
{
    ui->setupUi(this);

    /*---------------------------------------------------------*\
    | App translation                                           |
    | To add a new language:                                    |
    |   Add an entry to TRANSLATIONS in OpenRGB.pro             |
    |   Then run lupdate OpenRGB.pro to generate the new file   |
    |                                                           |
    | Edit this file with                                       |
    |   linguist qt/i18n/OpenRGB_en.ts qt/i18n/OpenRGB_XX.ts    |
    | or manually with any text editor                          |
    \*---------------------------------------------------------*/

    /*---------------------------------------------------------*\
    | Load available languages                                  |
    |  Tehcnically the QString is unused but declared           |
    |  here to show up in the translation file.                 |
    \*---------------------------------------------------------*/
    QTranslator translator;
    QMap<QString, QString> map;
    QString language = tr("English - US");

    QDirIterator file(":/i18n/", QDirIterator::Subdirectories);
    while(file.hasNext())
    {
        if(translator.load(file.next()))
        {
            map.insert(translator.translate("OpenRGBSettingsPage", "English - US"), file.filePath());
        }
    }

    ui->ComboBoxLanguage->blockSignals(true);
    ui->ComboBoxLanguage->addItem(tr("System Default"), "default");
    QMapIterator<QString, QString> i(map);
    while(i.hasNext())
    {
        i.next();
        ui->ComboBoxLanguage->addItem(i.key(), i.value());
    }
    ui->ComboBoxLanguage->blockSignals(false);


    /*--------------------------*\
    | Acquire lock on settings   |
    \*--------------------------*/
    SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();

    /*---------------------------------------------------------*\
    | Populate hex format combo box                             |
    \*---------------------------------------------------------*/
    ui->ComboBoxHexFormat->addItem("RGB");
    ui->ComboBoxHexFormat->addItem("BGR");

    hex_format_initialized = true;

    /*---------------------------------------------------------*\
    | Load theme settings                                       |
    \*---------------------------------------------------------*/
    ui->ComboBoxTheme->addItems({"Auto", "Light", "Dark"});

    const ThemeSettings& theme_settings = settings.access().theme;
    std::string theme;
    switch (theme_settings.theme)
    {
        case Theme::Light:
            theme = "Light";
            break;
        case Theme::Dark:
            theme = "Dark";
            break;
        case Theme::Auto:
            theme = "Auto";
            break;
    };

    ui->ComboBoxTheme->setCurrentText(QString::fromStdString(theme));
    theme_initialized = true;

    /*---------------------------------------------------------*\
    | Load user interface settings                              |
    \*---------------------------------------------------------*/
    const UserInterfaceSettings& ui_settings = settings.access().ui;
    /*-----------------------------------------------------*\
    | Get the language preference from settings             |
    |   and check the language combobox for a match         |
    \*-----------------------------------------------------*/
    int language_index = ui->ComboBoxLanguage->findText(QString::fromStdString(ui_settings.language));
    if(language_index > -1)
    {
        ui->ComboBoxLanguage->setCurrentIndex(language_index);
    }


    ui->CheckboxTrayIconGreyscale->setChecked(ui_settings.greyscale_tray_icon);
    ui->CheckboxMinimizeOnClose->setChecked(ui_settings.minimize_on_close);
    ui->CheckboxLoadGeometry->setChecked(ui_settings.geometry.load_geometry);
    ui->CheckboxSaveGeometry->setChecked(ui_settings.geometry.save_on_exit);
    ui->CheckboxRunZoneChecks->setChecked(ui_settings.run_zone_checks);
    ui->CheckboxDisableKeyExpansion->setChecked(ui_settings.disable_key_expansion);
    ui->CheckboxShowLEDView->setChecked(ui_settings.show_led_view);

    switch (ui_settings.hex_format)
    {
        case HexFormat::RGB:
            ui->ComboBoxHexFormat->setCurrentIndex(0);
            break;
        case HexFormat::BGR:
            ui->ComboBoxHexFormat->setCurrentIndex(1);
            break;
    }

    /*---------------------------------------------------------*\
    | Load LogManager settings                                  |
    \*---------------------------------------------------------*/
    const LogManagerSettings& log_manager_settings = settings.access().log_manager;
    ui->CheckboxLogFile->setChecked(log_manager_settings.log_file);
    ui->CheckboxLogConsole->setChecked(log_manager_settings.log_console);

    /*---------------------------------------------------------*\
    | Load detector settings                                    |
    \*---------------------------------------------------------*/
    const DetectorSettings& detector_settings = settings.access().detectors;
    ui->CheckboxHIDSafeMode->setChecked(detector_settings.hid_safe_mode);
    ui->TextDetectionDelay->setValue(detector_settings.initial_detection_delay_ms);

    /*---------------------------------------------------------*\
    | Load drivers settings (Windows only or Mac)               |
    \*---------------------------------------------------------*/
#if defined(WIN32) || defined(_MACOSX_X86_X64)
    const DriverSettings& drivers_settings = settings.access().drivers;
    ui->CheckboxAMDSMBusReduceCPU->setChecked(drivers_settings.amd_smbus_reduce_cpu);
#else
    ui->DriversSettingsLabel->hide();
    ui->CheckboxAMDSMBusReduceCPU->hide();
#endif
    /*---------------------------------------------------------*\
    | Load drivers settings (Windows only)                      |
    \*---------------------------------------------------------*/
#ifdef _WIN32
    ui->CheckboxSharedSMBusAccess->setChecked(drivers_settings.shared_smbus_access);
#else
    ui->CheckboxSharedSMBusAccess->hide();
#endif

    /*-----------------------------------------------------*\
    | Load server settings                                  |
    \*-----------------------------------------------------*/
    const ServerSettings& server_settings = settings.access().servers;

    ui->CheckboxAllDevices->setChecked(server_settings.all_devices);
    ui->CheckboxLegacyWorkaround->setChecked(server_settings.legacy_workaround);

    UpdateProfiles(settings);

    /*---------------------------------------------------------*\
    | Make sure autostart settings exist                        |
    \*---------------------------------------------------------*/
    CreateAutoStartSettings(settings);

    /*--------------------------------------------------------------------*\
    | Initialise UI to current settings or defaults                        |
      Remediate can modify the auto start settings, so we save just in case|
    \*--------------------------------------------------------------------*/
    AutoStartSettings& autostart_settings = settings.access_mut().auto_start;
    RemediateAutoStartProfile(autostart_settings);
    settings.Save();

    /*---------------------------------------------------------*\
    | Text boxes                                                |
    \*---------------------------------------------------------*/
    ui->TextServerHost->setText(QString::fromStdString(autostart_settings.host));
    ui->TextServerHost->setEnabled(autostart_settings.setserverhost);

    ui->TextServerPort->setValue(autostart_settings.port);
    ui->TextServerPort->setEnabled(autostart_settings.setserverportbool);

    ui->TextClientHost->setText(QString::fromStdString(autostart_settings.client));
    ui->TextClientHost->setEnabled(autostart_settings.setclient);

    ui->TextCustomArgs->setText(QString::fromStdString(autostart_settings.custom));
    ui->TextCustomArgs->setEnabled(autostart_settings.setcustom);

    /*---------------------------------------------------------*\
    | Checkboxes                                                |
    \*---------------------------------------------------------*/
    ui->CheckboxAutoStart->setChecked(autostart_settings.enabled);
    SetAutoStartVisibility(autostart_settings.enabled);

    ui->CheckboxAutoStartMinimized->setChecked(autostart_settings.setminimized);
    ui->CheckboxAutoStartClient->setChecked(autostart_settings.setclient);
    ui->CheckboxAutoStartServer->setChecked(autostart_settings.setserver);
    ui->CheckboxAutoStartSetServerHost->setChecked(autostart_settings.setserverhost);
    ui->CheckboxAutoStartSetServerPort->setChecked(autostart_settings.setserverportbool);
    ui->CheckboxAutoStartCustom->setChecked(autostart_settings.setcustom);

    ui->AutoStartStatusLabel->hide();
    autostart_initialized = true;
}

OpenRGBSettingsPage::~OpenRGBSettingsPage()
{
    delete ui;
}

void OpenRGBSettingsPage::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void OpenRGBSettingsPage::UpdateProfiles(SettingsAccessGuard& guard)
{
    /*-----------------------------------------------------*\
    | Load AutoStart settings                               |
    \*-----------------------------------------------------*/
    ProfileManager* profile_manager = ResourceManager::get()->GetProfileManager();

    /*-----------------------------------------------------*\
    | Load profiles into combo box                          |
    \*-----------------------------------------------------*/
    if(profile_manager != NULL)
    {
        ui->ComboBoxAutoStartProfile->blockSignals(true);
        ui->ComboBoxExitProfile->blockSignals(true);
        ui->ComboBoxOpenProfile->blockSignals(true);
        ui->ComboBoxResumeProfile->blockSignals(true);
        ui->ComboBoxSuspendProfile->blockSignals(true);

        ui->ComboBoxAutoStartProfile->clear();
        ui->ComboBoxExitProfile->clear();
        ui->ComboBoxOpenProfile->clear();
        ui->ComboBoxResumeProfile->clear();
        ui->ComboBoxSuspendProfile->clear();

        for(std::size_t profile_index = 0; profile_index < profile_manager->GetProfileList().size(); profile_index++)
        {
            QString new_profile = QString(profile_manager->GetProfileList()[profile_index].c_str());

            ui->ComboBoxAutoStartProfile->addItem(new_profile);
            ui->ComboBoxExitProfile->addItem(new_profile);
            ui->ComboBoxOpenProfile->addItem(new_profile);
            ui->ComboBoxResumeProfile->addItem(new_profile);
            ui->ComboBoxSuspendProfile->addItem(new_profile);
        }

        ui->ComboBoxAutoStartProfile->blockSignals(false);
        ui->ComboBoxExitProfile->blockSignals(false);
        ui->ComboBoxOpenProfile->blockSignals(false);
        ui->ComboBoxResumeProfile->blockSignals(false);
        ui->ComboBoxSuspendProfile->blockSignals(false);
    }

    /*-----------------------------------------------------*\
    | Load user interface settings                          |
    \*-----------------------------------------------------*/
    const AutoStartSettings& autostart_settings = guard.access().auto_start;

    /*-------------------------------------------------*\
    | Set the profile name from settings and check the  |
    |   profile combobox for a match                    |
    \*-------------------------------------------------*/
    int profile_index = ui->ComboBoxAutoStartProfile->findText(QString::fromStdString(autostart_settings.profile));

    if(profile_index > -1)
    {
        ui->ComboBoxAutoStartProfile->setCurrentIndex(profile_index);
    }

    /*-----------------------------------------------------*\
    | Load profile manager settings                         |
    \*-----------------------------------------------------*/
    const ProfileManagerSettings& profilemanager_settings = guard.access().profile_manager;

    /*--------------------------*\
    | Exit profile               |
    \*--------------------------*/
    ui->CheckboxSetOnExit->setChecked(profilemanager_settings.exit_profile.enabled);
    ui->ComboBoxExitProfile->setEnabled(profilemanager_settings.exit_profile.enabled);

    /*---------------------------------------------*\
    | Set the profile name from settings and check  |
    | the profile combobox for a match              |
    \*---------------------------------------------*/
    const std::string& exit_profile_name = profilemanager_settings.exit_profile.name;
    int exit_profile_index = ui->ComboBoxExitProfile->findText(QString::fromStdString(exit_profile_name));

    if(exit_profile_index > -1)
    {
        ui->ComboBoxExitProfile->setCurrentIndex(exit_profile_index);
    }

    /*--------------------------*\
    | Open profile               |
    \*--------------------------*/
    ui->CheckboxSetOnOpen->setChecked(profilemanager_settings.open_profile.enabled);
    ui->ComboBoxOpenProfile->setEnabled(profilemanager_settings.open_profile.enabled);

    /*---------------------------------------------*\
    | Set the profile name from settings and check  |
    | the profile combobox for a match              |
    \*---------------------------------------------*/
    const std::string& open_profile_name = profilemanager_settings.open_profile.name;
    int open_profile_index = ui->ComboBoxOpenProfile->findText(QString::fromStdString(open_profile_name));

    if(open_profile_index > -1)
    {
        ui->ComboBoxOpenProfile->setCurrentIndex(open_profile_index);
    }

    /*--------------------------*\
    | Resume profile               |
    \*--------------------------*/
    ui->CheckboxSetOnResume->setChecked(profilemanager_settings.resume_profile.enabled);
    ui->ComboBoxResumeProfile->setEnabled(profilemanager_settings.resume_profile.enabled);

    /*---------------------------------------------*\
    | Set the profile name from settings and check  |
    | the profile combobox for a match              |
    \*---------------------------------------------*/
    const std::string& resume_profile_name = profilemanager_settings.resume_profile.name;
    int resume_profile_index = ui->ComboBoxResumeProfile->findText(QString::fromStdString(resume_profile_name));

    if(resume_profile_index > -1)
    {
        ui->ComboBoxResumeProfile->setCurrentIndex(resume_profile_index);
    }

    /*--------------------------*\
    | Suspend profile               |
    \*--------------------------*/
    ui->CheckboxSetOnSuspend->setChecked(profilemanager_settings.suspend_profile.enabled);
    ui->ComboBoxSuspendProfile->setEnabled(profilemanager_settings.suspend_profile.enabled);

    /*---------------------------------------------*\
    | Set the profile name from settings and check  |
    | the profile combobox for a match              |
    \*---------------------------------------------*/
    const std::string& suspend_profile_name = profilemanager_settings.suspend_profile.name;
    int suspend_profile_index = ui->ComboBoxSuspendProfile->findText(QString::fromStdString(suspend_profile_name));

    if(suspend_profile_index > -1)
    {
        ui->ComboBoxSuspendProfile->setCurrentIndex(suspend_profile_index);
    }
}

void OpenRGBSettingsPage::on_ComboBoxLanguage_currentTextChanged(const QString language)
{

    bool loaded             = false;
    QString file            = ui->ComboBoxLanguage->currentData().toString();
    QApplication* app       = static_cast<QApplication *>(QApplication::instance());

    app->removeTranslator(&translator);

    if(file == "default")
    {
        QLocale locale = QLocale(QLocale::system());
        QLocale::setDefault(locale);

        loaded = translator.load(":/i18n/" + QString("OpenRGB_%1.qm").arg(locale.name()));
    }
    else
    {
        loaded = translator.load(file);
    }

    if(loaded)
    {
        app->installTranslator(&translator);
        LOG_DEBUG("[Settings] Changed Language to %s from the %s file\n", language.toStdString().c_str(), file.toStdString().c_str());

        SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
        UserInterfaceSettings& ui_settings = settings.access_mut().ui;
        if(ui_settings.language != language.toStdString())
        {
            ui_settings.language = language.toStdString();
            settings.Save();
        }
    }
}

void OpenRGBSettingsPage::on_ComboBoxTheme_currentTextChanged(const QString theme)
{
    if(theme_initialized)
    {
        SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
        ThemeSettings& theme_settings = settings.access_mut().theme;
        std::optional<Theme> parsed_theme = StringToTheme(theme.toStdString());

        if (parsed_theme)
        {
            theme_settings.theme = *parsed_theme;
            settings.Save();
        }
    }
}

void OpenRGBSettingsPage::on_ComboBoxHexFormat_currentTextChanged(const QString hex_format_str)
{
    if(hex_format_initialized)
    {
        SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
        UserInterfaceSettings& ui_settings = settings.access_mut().ui;
        std::optional<HexFormat> hex_format = StringToHexFormat(hex_format_str.toStdString());
        if (hex_format.has_value())
        {
            ui_settings.hex_format = hex_format.value();
            settings.Save();
        }
    }
}

void OpenRGBSettingsPage::on_CheckboxTrayIconGreyscale_clicked()
{
    bool checked = ui->CheckboxTrayIconGreyscale->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.ui.greyscale_tray_icon = checked;
    });
    emit TrayIconChanged(checked);
}

void OpenRGBSettingsPage::on_CheckboxMinimizeOnClose_clicked()
{
    bool checked = ui->CheckboxMinimizeOnClose->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.ui.minimize_on_close = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxLoadGeometry_clicked()
{
    bool checked = ui->CheckboxLoadGeometry->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.ui.geometry.load_geometry = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxSaveGeometry_clicked()
{
    bool checked = ui->CheckboxSaveGeometry->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.ui.geometry.load_geometry = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxRunZoneChecks_clicked()
{
    bool checked = ui->CheckboxRunZoneChecks->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.ui.run_zone_checks = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxSetOnExit_clicked(bool checked)
{
    SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
    ProfileManagerSettings& profilemanager_settings = settings.access_mut().profile_manager;
    profilemanager_settings.exit_profile = ProfileSettings {
        .enabled = checked,
        .name = ui->ComboBoxExitProfile->currentText().toStdString()
    };
    settings.Save();

    ui->ComboBoxExitProfile->setEnabled(checked);
}

void OpenRGBSettingsPage::on_ComboBoxExitProfile_currentTextChanged(const QString exit_profile_name)
{
    ResourceManager::get()->GetSettingsManager()->ModifySettings([exit_profile_name](Settings& settings) {
        settings.profile_manager.exit_profile.name = exit_profile_name.toStdString();
    });
}

void OpenRGBSettingsPage::on_CheckboxSetOnOpen_clicked(bool checked)
{
    SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
    ProfileManagerSettings& profilemanager_settings = settings.access_mut().profile_manager;
    profilemanager_settings.open_profile = ProfileSettings {
        .enabled = checked,
        .name = ui->ComboBoxOpenProfile->currentText().toStdString()
    };
    settings.Save();

    ui->ComboBoxOpenProfile->setEnabled(checked);
}

void OpenRGBSettingsPage::on_ComboBoxOpenProfile_currentTextChanged(const QString open_profile_name)
{
    ResourceManager::get()->GetSettingsManager()->ModifySettings([open_profile_name](Settings& settings) {
        settings.profile_manager.open_profile.name = open_profile_name.toStdString();
    });
}

void OpenRGBSettingsPage::on_CheckboxSetOnResume_clicked(bool checked)
{
    SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
    ProfileManagerSettings& profilemanager_settings = settings.access_mut().profile_manager;
    profilemanager_settings.resume_profile = ProfileSettings {
        .enabled = checked,
        .name = ui->ComboBoxResumeProfile->currentText().toStdString()
    };
    settings.Save();

    ui->ComboBoxResumeProfile->setEnabled(checked);
}

void OpenRGBSettingsPage::on_ComboBoxResumeProfile_currentTextChanged(const QString resume_profile_name)
{
    ResourceManager::get()->GetSettingsManager()->ModifySettings([resume_profile_name](Settings& settings) {
        settings.profile_manager.resume_profile.name = resume_profile_name.toStdString();
    });
}

void OpenRGBSettingsPage::on_CheckboxSetOnSuspend_clicked(bool checked)
{
    SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
    ProfileManagerSettings& profilemanager_settings = settings.access_mut().profile_manager;
    profilemanager_settings.suspend_profile = ProfileSettings {
        .enabled = checked,
        .name = ui->ComboBoxSuspendProfile->currentText().toStdString()
    };
    settings.Save();

    ui->ComboBoxSuspendProfile->setEnabled(checked);
}

void OpenRGBSettingsPage::on_ComboBoxSuspendProfile_currentTextChanged(const QString suspend_profile_name)
{
    ResourceManager::get()->GetSettingsManager()->ModifySettings([suspend_profile_name](Settings& settings) {
        settings.profile_manager.suspend_profile.name = suspend_profile_name.toStdString();
    });
}

void OpenRGBSettingsPage::on_CheckboxAllDevices_clicked(bool checked)
{
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.servers.all_devices = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxLegacyWorkaround_clicked(bool checked)
{
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.servers.legacy_workaround = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxAutoStart_clicked()
{
    if(autostart_initialized)
    {
        SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
        AutoStartSettings& autostart_settings = settings.access_mut().auto_start;
        autostart_settings.enabled = ui->CheckboxAutoStart->isChecked();

        if(autostart_settings.enabled)
        {
            RemediateAutoStartProfile(autostart_settings);
        }

        settings.Save();

        SetAutoStartVisibility(autostart_settings.enabled);

        ConfigureAutoStart(autostart_settings);
    }
}

void OpenRGBSettingsPage::on_CheckboxAutoStartMinimized_clicked()
{
    if (autostart_initialized)
    {
        bool checked = ui->CheckboxAutoStartMinimized->isChecked();
        ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
            settings.auto_start.setminimized = checked;
        });
    }
}

void OpenRGBSettingsPage::on_CheckboxAutoStartServer_clicked()
{
    if (autostart_initialized)
    {
        bool checked = ui->CheckboxAutoStartServer->isChecked();
        ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
            settings.auto_start.setserver = checked;
        });
    }
}

void OpenRGBSettingsPage::on_CheckboxAutoStartSetServerHost_clicked()
{
    if (autostart_initialized)
    {
        bool checked = ui->CheckboxAutoStartSetServerHost->isChecked();
        ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
            settings.auto_start.setserverhost = checked;
        });
        ui->TextServerHost->setEnabled(checked);
    }
}

void OpenRGBSettingsPage::on_CheckboxAutoStartSetServerPort_clicked()
{
    if (autostart_initialized)
    {
        bool checked = ui->CheckboxAutoStartSetServerPort->isChecked();
        ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
            settings.auto_start.setserverportbool = checked;
        });
        ui->TextServerHost->setEnabled(checked);
    }
}

void OpenRGBSettingsPage::on_CheckboxAutoStartClient_clicked()
{
    if (autostart_initialized)
    {
        bool checked = ui->CheckboxAutoStartClient->isChecked();
        ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
            settings.auto_start.setclient = checked;
        });
        ui->TextServerHost->setEnabled(checked);
    }
}

void OpenRGBSettingsPage::on_CheckboxAutoStartProfile_clicked()
{
    if (autostart_initialized)
    {
        bool checked = ui->CheckboxAutoStartProfile->isChecked();
        ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
            settings.auto_start.setprofile = checked;
        });
        ui->TextServerHost->setEnabled(checked);
    }
}

void OpenRGBSettingsPage::on_CheckboxAutoStartCustom_clicked()
{
    if (autostart_initialized)
    {
        bool checked = ui->CheckboxAutoStartCustom->isChecked();
        ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
            settings.auto_start.setcustom = checked;
        });
        ui->TextServerHost->setEnabled(checked);
    }
}

void OpenRGBSettingsPage::on_TextServerHost_textChanged(QString host)
{
    if (autostart_initialized)
    {
        ResourceManager::get()->GetSettingsManager()->ModifySettings([host](Settings& settings) {
            settings.auto_start.host = host.toStdString();
        });
    }
}

void OpenRGBSettingsPage::on_TextServerPort_valueChanged(int port)
{
    if (autostart_initialized)
    {
        ResourceManager::get()->GetSettingsManager()->ModifySettings([port](Settings& settings) {
            settings.auto_start.port = port;
        });
    }
}

void OpenRGBSettingsPage::on_TextClientHost_textChanged(QString client)
{
    if (autostart_initialized)
    {
        ResourceManager::get()->GetSettingsManager()->ModifySettings([client](Settings& settings) {
            settings.auto_start.client = client.toStdString();
        });
    }
}

void OpenRGBSettingsPage::on_TextCustomArgs_textChanged(QString custom)
{
    if (autostart_initialized)
    {
        ResourceManager::get()->GetSettingsManager()->ModifySettings([custom](Settings& settings) {
            settings.auto_start.custom = custom.toStdString();
        });
    }
}

void OpenRGBSettingsPage::on_ComboBoxAutoStartProfile_currentTextChanged(const QString profile)
{
    if (autostart_initialized)
    {
        ResourceManager::get()->GetSettingsManager()->ModifySettings([profile](Settings& settings) {
            settings.auto_start.profile = profile.toStdString();
        });
    }
}

void OpenRGBSettingsPage::SetAutoStartVisibility(bool visible)
{
    if (!visible)
    {
        ui->ComboBoxAutoStartProfile->hide();
        ui->CheckboxAutoStartClient->hide();
        ui->CheckboxAutoStartCustom->hide();
        ui->CheckboxAutoStartMinimized->hide();
        ui->CheckboxAutoStartProfile->hide();
        ui->CheckboxAutoStartServer->hide();
        ui->CheckboxAutoStartSetServerHost->hide();
        ui->CheckboxAutoStartSetServerPort->hide();
        ui->TextCustomArgs->hide();
        ui->TextClientHost->hide();
        ui->TextServerHost->hide();
        ui->TextServerPort->hide();
        ui->AutoStartStatusLabel->hide();
    }
    else
    {
        ui->ComboBoxAutoStartProfile->show();
        ui->CheckboxAutoStartClient->show();
        ui->CheckboxAutoStartCustom->show();
        ui->CheckboxAutoStartMinimized->show();
        ui->CheckboxAutoStartProfile->show();
        ui->CheckboxAutoStartServer->show();
        ui->CheckboxAutoStartSetServerHost->show();
        ui->CheckboxAutoStartSetServerPort->show();
        ui->TextCustomArgs->show();
        ui->TextClientHost->show();
        ui->TextServerHost->show();
        ui->TextServerPort->show();
        ui->AutoStartStatusLabel->show();
    }
}

void OpenRGBSettingsPage::ConfigureAutoStart(const AutoStartSettings& autostart_settings)
{
    AutoStart auto_start("OpenRGB");

    if (!autostart_settings.enabled)
    {
        if (!auto_start.DisableAutoStart())
        {
            ui->AutoStartStatusLabel->setText("A problem occurred disabling Start At Login.");
            ui->AutoStartStatusLabel->show();
            SetAutoStartVisibility(true);
        }
        else
        {
            ui->AutoStartStatusLabel->hide();
        }
    }
    else
    {
        std::string desc = "OpenRGB ";
        desc += VERSION_STRING;
        desc += ", for controlling RGB lighting.";


        std::stringstream arguments;

        if (autostart_settings.setminimized)
        {
            arguments << "--startminimized ";
        }

        if (autostart_settings.setserver)
        {
            arguments << "--server ";
        }

        if (autostart_settings.setserverhost)
        {
            arguments << "--server-host " << autostart_settings.host << " ";
        }

        if (autostart_settings.setserverportbool)
        {
            arguments << "--server-port " << autostart_settings.port << " ";
        }

        if (autostart_settings.setclient)
        {
            arguments << "--client " << autostart_settings.client << " ";

        }

        if (autostart_settings.setprofile)
        {
            arguments << "--profile " << autostart_settings.profile << " ";
        }

        if (autostart_settings.setcustom)
        {
            arguments << autostart_settings.custom << " ";
        }

        AutoStartInfo auto_start_info;

        auto_start_info.args        = arguments.str();
        auto_start_info.category    = "Utility;";
        auto_start_info.desc        = desc;
        auto_start_info.icon        = "OpenRGB";
        auto_start_info.path        = auto_start.GetExePath();

        if (!auto_start.EnableAutoStart(auto_start_info))
        {
            ui->AutoStartStatusLabel->setText(tr("A problem occurred enabling Start at Login."));
            ui->AutoStartStatusLabel->show();
            SetAutoStartVisibility(true);
        }
        else
        {
            ui->AutoStartStatusLabel->hide();
        }
    }
}

void OpenRGBSettingsPage::CreateAutoStartSettings(SettingsAccessGuard& guard)
{
    AutoStartSettings default_setting = AutoStartSettings
    {
        .enabled = false,
        .setminimized = false,
        .setprofile = false,
        .profile = ui->ComboBoxAutoStartProfile->count() > 0 ? ui->ComboBoxAutoStartProfile->itemText(0).toStdString(): "",
        .setclient = false,
        .client = "localhost:6742",
        .setserver = false,
        .setserverhost = false,
        .host = "0.0.0.0",
        .setserverportbool = false,
        .port = 6742,
        .setcustom = false,
        .custom = "",
    };

    AutoStartSettings& autostart_settings = guard.access_mut().auto_start;
    autostart_settings = default_setting;
    guard.Save();
}

void OpenRGBSettingsPage::RemediateAutoStartProfile(AutoStartSettings& autostart_settings)
{
    /*---------------------------------------------------------*\
    | If there are no profiles then we disable the UI for       |
    | profiles and if AutoStart is enabled then we force        |
    | disable setprofile                                        |
    \*---------------------------------------------------------*/
    if(ui->ComboBoxAutoStartProfile->count() == 0)
    {
        ui->CheckboxAutoStartProfile->setEnabled(false);
        ui->ComboBoxAutoStartProfile->setEnabled(false);

        /*--------------------------*\
        | restore default settings   |
        \*--------------------------*/
        autostart_settings.profile = "";

        if(autostart_settings.enabled)
        {
            autostart_settings.setprofile = false;
            ConfigureAutoStart(autostart_settings);
        }
    }

    /*---------------------------------------------------------*\
    | Else If the profile we want doesn't exist then we force   |
    | it to a profile which exists and if AutoStart is enabled  |
    | then we force disable setprofile                          |
    \*---------------------------------------------------------*/
    else if(autostart_settings.profile == "" ||
           (autostart_settings.profile != "" &&
            ui->ComboBoxAutoStartProfile->findText(QString::fromStdString(autostart_settings.profile)) == -1))
    {
        autostart_settings.profile = ui->ComboBoxAutoStartProfile->itemText(0).toStdString();

        if(autostart_settings.enabled)
        {
            autostart_settings.setprofile = false;

            ConfigureAutoStart(autostart_settings);
        }
    }

    ui->ComboBoxAutoStartProfile->setCurrentText(QString::fromStdString(autostart_settings.profile));
    ui->ComboBoxAutoStartProfile->setEnabled(autostart_settings.setprofile);
    ui->CheckboxAutoStartProfile->setChecked(autostart_settings.setprofile);
}

void OpenRGBSettingsPage::on_OpenSettingsFolderButton_clicked()
{
    std::string config_dir = ResourceManager::get()->GetConfigurationDirectory().generic_u8string();
    QUrl url = QUrl::fromLocalFile(QString::fromStdString(config_dir));
    QDesktopServices::openUrl(url);
}


void OpenRGBSettingsPage::on_CheckboxLogConsole_clicked()
{
    bool checked = ui->CheckboxLogConsole->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.log_manager.log_console = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxLogFile_clicked()
{
    bool checked = ui->CheckboxLogFile->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.log_manager.log_file = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxHIDSafeMode_clicked()
{
    bool checked = ui->CheckboxHIDSafeMode->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.detectors.hid_safe_mode = checked;
    });
}

void OpenRGBSettingsPage::on_TextDetectionDelay_valueChanged(int delay)
{
    ResourceManager::get()->GetSettingsManager()->ModifySettings([delay](Settings& settings) {
        settings.detectors.initial_detection_delay_ms = delay;
    });
}

void OpenRGBSettingsPage::on_CheckboxAMDSMBusReduceCPU_clicked()
{
    bool checked = ui->CheckboxAMDSMBusReduceCPU->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.drivers.amd_smbus_reduce_cpu = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxSharedSMBusAccess_clicked()
{
    bool checked = ui->CheckboxSharedSMBusAccess->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.drivers.shared_smbus_access = checked;
    });
}

void OpenRGBSettingsPage::on_CheckboxDisableKeyExpansion_clicked()
{
    bool checked = ui->CheckboxDisableKeyExpansion->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.ui.disable_key_expansion = checked;
    });
}

void  OpenRGBSettingsPage::on_CheckboxShowLEDView_clicked()
{
    bool checked = ui->CheckboxShowLEDView->isChecked();
    ResourceManager::get()->GetSettingsManager()->ModifySettings([checked](Settings& settings) {
        settings.ui.show_led_view = checked;
    });
}
