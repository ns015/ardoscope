#include "mainwindow.h"
#include "runtimedialog.h"
#include "ui_runtimedialog.h"

#include <QSettings>

RuntimeDialog::RuntimeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RuntimeDialog)
{
    ui->setupUi(this);
    connect(ui->applyButton, &QPushButton::clicked,
            this, &RuntimeDialog::apply);
    connect(ui->defaultButton, &QPushButton::clicked,
            this, &RuntimeDialog::default_values);
    readSettings();
}

RuntimeDialog::~RuntimeDialog()
{
    writeSettings();

    delete ui;
}

void RuntimeDialog::readSettings()
{
    QSettings settings(QCoreApplication::applicationName(), m_settings_file_name);
    qint32 R1 = settings.value("R1", defaultSettings_R1).toInt();
    qint32 R2 = settings.value("R2", defaultSettings_R2).toInt();
    qint32 R3 = settings.value("R3", defaultSettings_R3).toInt();
    double Vref = settings.value("Uref", defaultSettings_Uref).toDouble();

    ui->R1lineEdit->setText(QString::number(R1));
    ui->R2lineEdit->setText(QString::number(R2));
    ui->R3lineEdit->setText(QString::number(R3));
    ui->UreflineEdit->setText(QString::number(Vref));

    updateSettingsStructure();
}

void RuntimeDialog::writeSettings()
{
    QSettings settings(QCoreApplication::applicationName(), m_settings_file_name);
    settings.setValue("R1", m_currentSettings.R1);
    settings.setValue("R2", m_currentSettings.R2);
    settings.setValue("R3", m_currentSettings.R3);
    settings.setValue("Uref", m_currentSettings.Uref);
}

RuntimeDialog::Settings *RuntimeDialog::psettings()
{
    return &m_currentSettings;
}

void RuntimeDialog::updateSettingsStructure()
{
    m_currentSettings.R1 = ui->R1lineEdit->text().toUInt();
    m_currentSettings.R2 = ui->R2lineEdit->text().toUInt();
    m_currentSettings.R3 = ui->R3lineEdit->text().toUInt();
    m_currentSettings.Uref = ui->UreflineEdit->text().toDouble();

    const double Rin = m_currentSettings.R1;
    const double R1A0 = m_currentSettings.R2;
    const double R2A0 = m_currentSettings.R3;
    const double Uref = m_currentSettings.Uref;
    const double Rcabel = 100;

    m_currentSettings.Uin1 = -Uref*(Rin + Rcabel)/R1A0;
    m_currentSettings.Uin2 = Uref*((Rin + Rcabel + R2A0)/R2A0);


    double R = (Rin+Rcabel)*R2A0/(Rin+Rcabel+R2A0);
    m_currentSettings.CodeUin0 = 1024.*R/(R1A0 + R);

    ui->U1U2->setText(tr("U1=%1(V), U2=%2(V), CodeU0=%3").arg(QString::number(m_currentSettings.Uin1))
                      .arg(QString::number(m_currentSettings.Uin2))
                      .arg(QString::number(m_currentSettings.CodeUin0)));
}

void RuntimeDialog::apply()
{
    updateSettingsStructure();
    emit update_delay();
}

void RuntimeDialog::default_values()
{
    ui->R1lineEdit->setText(QString::number(defaultSettings_R1));
    ui->R2lineEdit->setText(QString::number(defaultSettings_R2));
    ui->R3lineEdit->setText(QString::number(defaultSettings_R3));
    ui->UreflineEdit->setText(QString::number(defaultSettings_Uref));
    updateSettingsStructure();
}
