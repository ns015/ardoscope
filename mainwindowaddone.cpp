#include "mainwindow.h"
#include "settingsdialog.h"
#include "runtimedialog.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTextStream>
#include <QPicture>
#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSettings>
#include <math.h>
#ifdef Q_OS_WIN32
# include <windows.h>
#else
#  include <X11/XKBlib.h>
# undef KeyPress
# undef KeyRelease
# undef FocusIn
# undef FocusOut
// #undef those Xlib #defines that conflict with QEvent::Type enum
#endif
bool MainWindow::checkCapsLock()
{
 // platform dependent method of determining if CAPS LOCK is on
#ifdef Q_OS_WIN32 // MS Windows version
 return GetKeyState(VK_CAPITAL) == 1;
#else // X11 version (Linux/Unix/Mac OS X/etc...)
 Display * d = XOpenDisplay((char*)0);
 bool caps_state = false;
 if (d)
 {
  unsigned n;
  XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
  caps_state = (n & 0x01) == 1;
  XCloseDisplay(d);
 }
 return caps_state;
#endif
}
bool MainWindow::checkNumLock()
{
#ifdef _WIN32
    short status = GetKeyState(VK_NUMLOCK);
    return status == 1;
#endif

#ifndef Q_OS_WIN32
    bool num_lock_status = false;
    Display *dpy = XOpenDisplay((char*)0);
    if (dpy) {
        XKeyboardState x;
        XGetKeyboardControl(dpy, &x);
        XCloseDisplay(dpy);
        num_lock_status = x.led_mask & 2;
    }
    return num_lock_status;
#endif
}
void MainWindow::update_V(void)
{
    double val = m_V_per_mark[m_V_per_mark_index];
    QString buf;
    if (val < 0.1) {
        buf = tr("%1 mV/mark")
                .arg(QString::number(val*1000.));
    } else {
        buf = tr("%1 V/mark")
                .arg(QString::number(val));
    }
    ui->m_U->setText(tr("%1").arg(buf));
}
void MainWindow::update_T()
{
    QString str_T_per_mark;
    if (m_T_per_mark[m_T_per_mark_index] > 0.1) {
        str_T_per_mark = tr("%1 sec/mark").arg(QString::number(m_T_per_mark[m_T_per_mark_index]));
    } else if (m_T_per_mark[m_T_per_mark_index] > 0.1/1000) {
        str_T_per_mark = tr("%1 msec/mark").arg(QString::number(m_T_per_mark[m_T_per_mark_index]*1000.));
    } else {
        str_T_per_mark = tr("%1 usec/mark").arg(QString::number(m_T_per_mark[m_T_per_mark_index]*1000.*1000.));
    };
    if (m_control_T_flag) {
        ui->m_T->setText(tr("%1 lock").arg(str_T_per_mark));
    } else {
        ui->m_T->setText(tr("%1").arg(str_T_per_mark));
    }
}

void MainWindow::update_Fr()
{
    QString str_Fr;
    if (m_Fr > 1000) {
        str_Fr = tr("%1(KHz)").arg(QString::number(m_Fr/1000));
    } else {
        str_Fr = tr("%1(Hz)").arg(QString::number(m_Fr));
    };
    ui->m_Frequency->setText(tr("%1").arg(str_Fr));
}


double MainWindow::getNewOfsetY(int new_index)
{
    if (m_V_per_mark_index < (int)(sizeof(m_V_per_mark)/sizeof(m_V_per_mark[0]))) {
        double offset = (m_offset_val_y - (pB - pT) / 2. - pT) * m_V_per_mark[m_V_per_mark_index];
        if (new_index <  (int)(sizeof(m_V_per_mark)/sizeof(m_V_per_mark[0]))) {
            return offset / m_V_per_mark[new_index] + (pB - pT) / 2. + pT;
        } else {
            qDebug()<<"error"<<new_index;
            return 0;
        }
    } else {
        qDebug()<<"error"<<m_V_per_mark_index;
        return 0;
    }
}
double MainWindow::getNewOfsetX(int new_index)
{
    if (m_T_per_mark_index < (int)(sizeof(m_T_per_mark)/sizeof(m_T_per_mark[0]))) {
        double offset = (m_offset_val_x - (double)(pR - pL) / 2. - pL) * m_T_per_mark[m_T_per_mark_index];
        if (new_index <  (int)(sizeof(m_T_per_mark)/sizeof(m_T_per_mark[0]))) {
            offset = offset / m_T_per_mark[new_index] + (double)(pR - pL) / 2. + pL;
            return offset;
        } else {
            qDebug()<<"error"<<new_index;
            return 0;
        }
    } else {
        qDebug()<<"error"<<m_V_per_mark_index;
        return 0;
    }
}
void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::applicationName(), m_settings_file_name);
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    int def_V_Index = sizeof(m_V_per_mark)/sizeof(m_V_per_mark[0]) - 1;
    quint32 V_index = settings.value("Vindex", def_V_Index ).toInt();
    quint32 T_index = settings.value("Tindex", 0).toInt();

    resize(size);
    move(pos);

    if (V_index < sizeof(m_V_per_mark)/sizeof(m_V_per_mark[0])) {
        m_V_per_mark_index = V_index;
    } else {
        m_V_per_mark_index = def_V_Index;
    }
    if (T_index < sizeof(m_T_per_mark)/sizeof(m_T_per_mark[0])) {
        m_T_per_mark_index = T_index;
    } else {
        m_T_per_mark_index = 0;
    }

    m_offset_val_y = settings.value("Voffset", (pB - pT)/2 + pT).toDouble();
    m_offset_val_x = settings.value("Toffset", pL).toDouble();

    bool done = true;
    int i;
    int ResponseCount = settings.value("ResponseCount").toInt();
    double StepT = settings.value("StepT").toDouble();

    MyTypeSelector mytype = static_cast<MainWindow::MyTypeSelector>(settings.value("ResponseType").toInt());
    QStringList slY, slYmin, slYmax;

    if (mytype == Bar) {
        slYmax = settings.value("ResponseYmax").toStringList();
        slYmin = settings.value("ResponseYmin").toStringList();
    } else if (mytype == Line) {
        slY = settings.value("ResponseY").toStringList();
    }

    if (mytype == Line) {
        if (ResponseCount != slY.count()) {
            done = false;
        }

    } else if (mytype == Bar) {
        if ( (ResponseCount != slYmax.count())
           || (ResponseCount != slYmin.count()) ) {
            done = false;
        }
    } else {
        done = false;
    }
    if (done) {
        m_result.ts = mytype;
        m_result.step_t = StepT;
        m_result.vector.resize(ResponseCount);
        for (i=0;i<ResponseCount;i++) {
            MyPoint *mp = &m_result.vector[i];
            memset(mp, 0, sizeof (*mp));
            if (mytype == Bar) {
                mp->max_y = slYmax.at(i).toDouble();
                mp->min_y = slYmin.at(i).toDouble();
            } else {
                mp->Y = slY.at(i).toDouble();
            }
        }
    }
    m_trigger.tr_type = static_cast<MainWindow::MyTypeTrigger>(settings.value("TriggerType").toInt());
    m_trigger.code = settings.value("TriggerLevel").toInt();
    if (m_trigger.tr_type == Positive) {
        ui->actionNegative->setChecked(false);
        ui->actionPositive->setChecked(true);
        ui->actionNo_trigger->setChecked(false);
    } else if (m_trigger.tr_type == Negative) {
        ui->actionNegative->setChecked(true);
        ui->actionPositive->setChecked(false);
        ui->actionNo_trigger->setChecked(false);
    } else {
        ui->actionNegative->setChecked(false);
        ui->actionPositive->setChecked(false);
        ui->actionNo_trigger->setChecked(true);
    }
    m_Fr = settings.value("Fr", 0).toDouble();
    update_Fr();
}
void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::applicationName(), m_settings_file_name);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("Vindex", m_V_per_mark_index);
    settings.setValue("Tindex", m_T_per_mark_index);
    settings.setValue("Voffset", m_offset_val_y);
    settings.setValue("Toffset", m_offset_val_x);
    int i;
    int ResponseCount = m_result.vector.count();
    settings.setValue("ResponseCount", ResponseCount);
    settings.setValue("ResponseType", m_result.ts);
    settings.setValue("StepT", m_result.step_t);

    QStringList sl;

    sl.clear();
    for (i=0;i<ResponseCount;i++)
        sl.append(QString::number(m_result.vector.at(i).Y));
    settings.setValue("ResponseY", sl);

    sl.clear();
    for (i=0;i<ResponseCount;i++)
        sl.append(QString::number(m_result.vector.at(i).max_y));
    settings.setValue("ResponseYmax", sl);
    sl.clear();
    for (i=0;i<ResponseCount;i++)
        sl.append(QString::number(m_result.vector.at(i).min_y));
    settings.setValue("ResponseYmin", sl);

    settings.setValue("TriggerType", m_trigger.tr_type);
    settings.setValue("TriggerLevel", (int)m_trigger.code);
    settings.setValue("Fr", m_Fr);
}
