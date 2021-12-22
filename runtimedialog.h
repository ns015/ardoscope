#ifndef RUNTIMEDIALOG_H
#define RUNTIMEDIALOG_H

#include <QDialog>

namespace Ui {
class RuntimeDialog;
}

class RuntimeDialog : public QDialog
{
    Q_OBJECT

signals:
    void update_delay(void);

public:

    struct Settings {
        quint32 R1;
        quint32 R2;
        quint32 R3;
        double Uref;
        double Uin1;
        double Uin2;
        double CodeUin0;
    };

    explicit RuntimeDialog(QWidget *parent = nullptr);
    ~RuntimeDialog();

    Settings *psettings();

private slots:
    void apply(void);
    void default_values(void);

private:
    void readSettings(void);
    void writeSettings(void);
    void fillParameters(void);
    void updateSettingsStructure(void);


private:
    Ui::RuntimeDialog *ui;
    Settings m_currentSettings;
   const QString m_settings_file_name = "runtimesettings";

   const quint32 defaultSettings_R1 = 9060;
   const quint32 defaultSettings_R2 = 1483;
   const quint32 defaultSettings_R3 = 1484;
   const double defaultSettings_Uref = 2.495;
};

#endif // RUNTIMEDIALOG_H
