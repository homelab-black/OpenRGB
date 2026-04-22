
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include "OpenRGBDynamicSettingsWidget.h"
#include "ProfileManager.h"
#include "ResourceManager.h"

OpenRGBDynamicSettingsWidget::OpenRGBDynamicSettingsWidget(std::string key, nlohmann::json schema, nlohmann::json settings, QWidget* parent) :
    QWidget(parent)
{
    this->key       = key;
    this->schema    = schema;
    this->settings  = settings;
    callback = nullptr;

    /*-----------------------------------------------------*\
    | Ensure each schema entry has a title and type         |
    \*-----------------------------------------------------*/
    if(schema.contains("title") && schema.contains("type"))
    {
        /*-------------------------------------------------*\
        | Create main layout                                |
        \*-------------------------------------------------*/
        layout                              = new QHBoxLayout(this);
        layout->setMargin(0);

        /*-------------------------------------------------*\
        | Get schema title and type, check if it is an enum |
        \*-------------------------------------------------*/
        std::string         title           = schema["title"];
        std::string         type            = schema["type"];
        bool                is_enum         = false;

        if(schema.contains("enum"))
        {
            is_enum                         = true;
        }

        /*-------------------------------------------------*\
        | If type is object, create a nested group          |
        \*-------------------------------------------------*/
        if(type == "object" && schema.contains("properties"))
        {
            /*---------------------------------------------*\
            | Create the groupbox and layout                |
            \*---------------------------------------------*/
            nlohmann::json&     properties_json = schema["properties"];
            nlohmann::json&     settings_json   = settings[key];
            QGroupBox*          groupbox        = new QGroupBox(QString::fromStdString(title));
            QVBoxLayout*        groupbox_layout = new QVBoxLayout();

            groupbox->setLayout(groupbox_layout);

            for(nlohmann::json::iterator json_iterator = properties_json.begin(); json_iterator != properties_json.end(); json_iterator++)
            {
                nlohmann::json&                 property    = json_iterator.value();
                OpenRGBDynamicSettingsWidget*   item_widget = new OpenRGBDynamicSettingsWidget(json_iterator.key(), property, settings_json);

                groupbox_layout->addWidget(item_widget);
            }

            layout->addWidget(groupbox);
        }
        /*-------------------------------------------------*\
        | Otherwise, create a single settings entry         |
        \*-------------------------------------------------*/
        else
        {
            /*---------------------------------------------*\
            | Create the layout and label                   |
            \*---------------------------------------------*/
            label                               = new QLabel(QString::fromStdString(title));

            /*---------------------------------------------*\
            | Add the title label                           |
            \*---------------------------------------------*/
            layout->addWidget(label, 1);

            /*---------------------------------------------*\
            | Add a LineEdit for non-enum string settings   |
            \*---------------------------------------------*/
            if(type == "string" && !is_enum)
            {
                std::string     value           = "";

                if(settings.contains(key))
                {
                    value                       = settings[key];
                }

                widget                          = (QWidget*)new QLineEdit(QString::fromStdString(value));

                QObject::connect((QLineEdit*)widget, &QLineEdit::textChanged, this, &OpenRGBDynamicSettingsWidget::OnSettingChanged);
            }
            /*---------------------------------------------*\
            | Add a ComboBox for enum string settings       |
            \*---------------------------------------------*/
            else if(type == "string" && is_enum)
            {
                std::string     value           = "";

                if(settings.contains(key))
                {
                    value                       = settings[key];
                }

                widget                          = (QWidget*)new QComboBox();

                for(std::size_t enum_idx = 0; enum_idx < schema["enum"].size(); enum_idx++)
                {
                    std::string enum_str        = schema["enum"][enum_idx];
                    ((QComboBox*)widget)->addItem(QString::fromStdString(enum_str));
                }

                ((QComboBox*)widget)->setCurrentText(QString::fromStdString(value));

                QObject::connect((QComboBox*)widget, qOverload<int>(&QComboBox::currentIndexChanged), this, &OpenRGBDynamicSettingsWidget::OnSettingChanged);
            }
            /*---------------------------------------------*\
            | Add a LineEdit for non-enum integer settings  |
            \*---------------------------------------------*/
            if(type == "integer" && !is_enum)
            {
                int             value           = 0;

                if(settings.contains(key))
                {
                    value                       = settings[key];
                }

                widget                          = (QWidget*)new QLineEdit(QString::number(value));

                QObject::connect((QLineEdit*)widget, &QLineEdit::textChanged, this, &OpenRGBDynamicSettingsWidget::OnSettingChanged);
            }
            /*---------------------------------------------*\
            | Add a ComboBox for enum integer settings      |
            \*---------------------------------------------*/
            else if(type == "integer" && is_enum)
            {
                int             value           = 0;

                if(settings.contains(key))
                {
                    value                       = settings[key];
                }

                widget                          = (QWidget*)new QComboBox();

                for(std::size_t enum_idx = 0; enum_idx < schema["enum"].size(); enum_idx++)
                {
                    int         enum_int        = schema["enum"][enum_idx];
                    ((QComboBox*)widget)->addItem(QString::number(enum_int));
                }

                ((QComboBox*)widget)->setCurrentText(QString::number(value));

                QObject::connect((QComboBox*)widget, qOverload<int>(&QComboBox::currentIndexChanged), this, &OpenRGBDynamicSettingsWidget::OnSettingChanged);
            }
            /*---------------------------------------------*\
            | Add a CheckBox for bool settings              |
            \*---------------------------------------------*/
            else if(type == "bool")
            {
                bool            value           = false;

                if(settings.contains(key))
                {
                    value                       = settings[key];
                }

                widget                          = (QWidget*)new QCheckBox();

                ((QCheckBox*)widget)->setChecked(value);

                QObject::connect((QCheckBox*)widget, &QCheckBox::stateChanged, this, &OpenRGBDynamicSettingsWidget::OnSettingChanged);
            }
            /*---------------------------------------------*\
            | Add a ComboBox for profile settings           |
            \*---------------------------------------------*/
            else if(type == "profile")
            {
                std::string                 value           = "";
                std::vector<std::string>    profile_list    =  ResourceManager::get()->GetProfileManager()->GetProfileList();

                //if(settings.contains(key))
                //{
                //    value                                   = settings[key];
                //}

                widget                                      = (QWidget*)new QComboBox();

                for(std::size_t profile_idx = 0; profile_idx < profile_list.size(); profile_idx++)
                {
                    std::string             profile_str     = profile_list[profile_idx];
                    ((QComboBox*)widget)->addItem(QString::fromStdString(profile_str));
                }

                ((QComboBox*)widget)->setCurrentText(QString::fromStdString(value));

                QObject::connect((QComboBox*)widget, qOverload<int>(&QComboBox::currentIndexChanged), this, &OpenRGBDynamicSettingsWidget::OnSettingChanged);
            }

            layout->addWidget(widget, 1);
            adjustSize();
        }
    }
}

OpenRGBDynamicSettingsWidget::~OpenRGBDynamicSettingsWidget()
{

}

void OpenRGBDynamicSettingsWidget::OnSettingChanged()
{
    nlohmann::json          settings_change;

    /*-----------------------------------------------------*\
    | Ensure each schema entry has a title and type         |
    \*-----------------------------------------------------*/
    if(schema.contains("title") && schema.contains("type"))
    {
        /*-------------------------------------------------*\
        | Get schema title and type, check if it is an enum |
        \*-------------------------------------------------*/
        std::string         title           = schema["title"];
        std::string         type            = schema["type"];
        bool                is_enum         = false;

        if(schema.contains("enum"))
        {
            is_enum                         = true;
        }

        if(type == "string" && !is_enum)
        {
            std::string     value           = ((QLineEdit*)widget)->text().toStdString();
            settings_change[key]            = value;
        }
        else if(type == "string" && is_enum)
        {
            std::string     value           = ((QComboBox*)widget)->currentText().toStdString();
            settings_change[key]            = value;
        }
        if(type == "integer" && !is_enum)
        {
            int             value           = ((QLineEdit*)widget)->text().toInt();
            settings_change[key]            = value;
        }
        else if(type == "integer" && is_enum)
        {
            int             value           = ((QComboBox*)widget)->currentText().toInt();
            settings_change[key]            = value;
        }
        else if(type == "bool")
        {
            bool            value           = ((QCheckBox*)widget)->isChecked();
            settings_change[key]            = value;
        }
    }

    if(callback)
    {
        callback(callback_arg, key, settings_change);
    }
}

void OpenRGBDynamicSettingsWidget::SetCallback(OpenRGBDynamicSettingsCallback callback, void* callback_arg)
{
    this->callback      = callback;
    this->callback_arg  = callback_arg;
}
