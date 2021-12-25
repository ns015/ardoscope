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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_status(new QLabel())
    , m_serial(new QSerialPort(this))
    , m_settings(new SettingsDialog(this))
    , m_rtsettings(new RuntimeDialog(this))
{
    ui->setupUi(this);

    ui->statusbar->addWidget(m_status);

    togleState(true);

    initActionsConnections();

    m_pi = new QPicture();
    DrawGrid(m_pi);

    connect(ui->m_runButton, &QPushButton::clicked, this, &MainWindow::transaction);

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(transaction()));
    m_op_timer = new QTimer(this);
    connect(m_op_timer, SIGNAL(timeout()), this, SLOT(op_timeout()));

    ui->m_Display->installEventFilter(this);
    ui->m_T->installEventFilter(this);
    ui->m_U->installEventFilter(this);

    readSettings();

    update_V();
    update_T();

    int i;
    double step_x1, step_x2;
    step_x2 = 1/Fsamp;

    for(i=0;i<(int)(sizeof(m_T_per_mark)/sizeof(m_T_per_mark[0]));i++) {
        step_x1 = m_T_per_mark[i]/((pR-pL)/nX);
        m_T_decimation[i] = step_x1/step_x2;
    };
    DrawResult();
    /* Создаем объект контекстного меню */
    m_cnt_menu = new QMenu(this);
    /* Подключаем СЛОТы обработчики для действий контекстного меню */
    connect(ui->actionPositive, SIGNAL(triggered(bool)), this, SLOT(slotPositiveTrigger(bool)));
    connect(ui->actionNegative, SIGNAL(triggered(bool)), this, SLOT(slotNegativeTrigger(bool)));
    connect(ui->actionNo_trigger, SIGNAL(triggered(bool)), this, SLOT(slotFreeRunningTrigger(bool)));
    /* Устанавливаем действия в меню */
    m_cnt_menu->addAction(ui->actionPositive);
    m_cnt_menu->addAction(ui->actionNegative);
    m_cnt_menu->addAction(ui->actionNo_trigger);
    connect(ui->m_Display, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomMenuRequested(QPoint)));
}

MainWindow::~MainWindow()
{
    ui->m_U->removeEventFilter(this);
    ui->m_T->removeEventFilter(this);
    ui->m_Display->removeEventFilter(this);

    m_timer->stop();
    m_op_timer->stop();

    writeSettings();

    delete m_settings;
    delete m_rtsettings;
    delete ui;
    delete m_pi;
}

void MainWindow::print(const QString &s)
{
    ui->m_textEdit->append(s);
}

void MainWindow::processError(const QString &s)
{
    print(s);
}

void MainWindow::processTimeout(const QString &s)
{
    print(s);
}

void MainWindow::on_m_runButton_clicked()
{
    if (!m_timer->isActive()) {
        m_timer->start(100);
        ui->m_runButton->setText("Stop");
    } else {
        m_timer->stop();
        ui->m_runButton->setText("Run");
    }
}
void MainWindow::DrawGrid(QPicture *pi)
{
    const int stepX = (pR - pL) / nX;
    const int stepY = (pB - pT) / nY;
    int i, x, y;
    QPainter p(pi);
    p.setRenderHint(QPainter::Antialiasing);
    for (i=0;i<=nX;i++) {
        if (i==nX/2) {
            p.setPen(QPen(Qt::gray, 0.8, Qt::SolidLine, Qt::RoundCap));
        } else {
            p.setPen(QPen(Qt::gray, 0.5, Qt::SolidLine, Qt::RoundCap));
        }
        x = pL + stepX * i;
        p.drawLine( x, pT, x, pB); // вертик.
    }
    for (i=0;i<=nY;i++) {
        if (i==nY/2) {
            p.setPen(QPen(Qt::gray, 0.8, Qt::SolidLine, Qt::RoundCap));
        } else {
            p.setPen(QPen(Qt::gray, 0.5, Qt::SolidLine, Qt::RoundCap));
        }
        y = pT + stepY * i;
        p.drawLine(pL, y, pR, y); // гориз.
    }
    p.end();
}
bool MainWindow::trimXY(QPoint &pos)
{
    if (pos.y()<pT+1) pos.setY(pT+1);
    if (pos.y()>pB-1) pos.setY(pB-1);
    if (pos.x()<pL+1) pos.setX(pL+1);
    if (pos.x()>pR-1) pos.setX(pR-1);
    return true;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel)
    {
        QWheelEvent *mouseEvent = static_cast<QWheelEvent*>(event);
        QPoint numDegrees = mouseEvent->angleDelta() / 8;

        bool ctrl_flag = false;
        bool shift_flag = false;
        if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        {
            ctrl_flag = true;
        }
        if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
        {
            shift_flag = true;
        }

        int sign_y;

        if (numDegrees.y()>0) {
            sign_y = 1;
        } else {
            sign_y =-1;
        }

        if (obj == ui->m_Display) {
            if (ctrl_flag) {
                if (shift_flag) {
                    m_offset_val_x += sign_y;
                    DrawResult();
                } else {
                    m_offset_val_x += sign_y * (pB - pT)/ nY;
                    DrawResult();
                }
            } else {
                if (shift_flag) {
                    m_offset_val_y += -sign_y;
                    DrawResult();
                } else {
                    m_offset_val_y += -sign_y * (pR - pL)/ nX;
                    DrawResult();
                }
            }
        }
        if (obj == ui->m_T) {
            if (ctrl_flag) {
                if (sign_y>0) {
                    if (m_T_per_mark_index < (int) (sizeof(m_T_per_mark)/sizeof(m_T_per_mark[0]) - 1)) {
                        m_T_per_mark_index++;
                        update_delay();
                        update_T();
                        DrawResult();
                    }
                } else {
                  if (m_T_per_mark_index > 0) {
                    m_T_per_mark_index--;
                    update_delay();
                    update_T();
                    DrawResult();
                    }
                }
            } else {
                if (shift_flag) {
                    m_offset_val_x += sign_y;
                    DrawResult();
                } else {
                    m_offset_val_x += sign_y * (pB - pT)/ nY;
                    DrawResult();
                }
            }
        }
        if (obj == ui->m_U) {
            if (sign_y > 0) {
                if (m_V_per_mark_index < (int) (sizeof(m_V_per_mark)/sizeof(m_V_per_mark[0]) - 1)) {
                    m_offset_val_y = getNewOfsetY(m_V_per_mark_index + 1);
                    m_V_per_mark_index++;
                    update_V();
                    DrawResult();
                }
            } else {
                if (m_V_per_mark_index > 0) {
                    m_offset_val_y = getNewOfsetY(m_V_per_mark_index - 1);
                    m_V_per_mark_index--;
                    update_V();
                    DrawResult();
                }
            }
        }
    } else if (event->type() == QEvent::MouseButtonDblClick)
    {
        if (obj == ui->m_Display) {
            if ( m_result.vector.size() == 0) { return true; }
            int i;
            int max_y = -10000;
            int min_y = 10000;
            if (m_result.ts == Line) {
                for (i=0;i<m_result.vector.size();i++) {
                    if (max_y < m_result.vector[i].Y) {
                        max_y = m_result.vector[i].Y;
                    }
                    if (min_y > m_result.vector[i].Y) {
                        min_y = m_result.vector[i].Y;
                    }
                }
            } else if (m_result.ts == Bar) {
                for (i=0;i<m_result.vector.size();i++) {
                    if (max_y < m_result.vector[i].max_y) {
                        max_y = m_result.vector[i].max_y;
                    }
                    if (min_y > m_result.vector[i].min_y) {
                        min_y = m_result.vector[i].min_y;
                    }
                }
            }
            double Umax = get_U_from_code(max_y);
            double Umin = get_U_from_code(min_y);
            double U_per_mark = (Umax - Umin) / nY;
            double U_per_mark_abs = fabs(U_per_mark);
            for (i=0;i<(int)(sizeof(m_V_per_mark)/sizeof(m_V_per_mark[0]));i++) {
                if (m_V_per_mark[i] >= U_per_mark_abs) {
                    m_V_per_mark_index = i;
                    break;
                }
            }
            double mid_code = (double)(max_y - min_y)/2. + min_y;
            m_offset_val_y = get_Y_px(mid_code) + (pB - pT) / 2 + pT;

            double range_T = m_result.step_t * m_result.vector.count();
            double range_T_per_mark = range_T / nX;

            for (i=0;i<(int)(sizeof(m_T_per_mark)/sizeof(m_T_per_mark[0]));i++) {
                if (m_T_per_mark[i] >= range_T_per_mark) {
                    m_T_per_mark_index = i;
                    break;
                }
            }
            double range_T2 = m_T_per_mark[m_T_per_mark_index]*nX;
            m_offset_val_x = ((pR - pL) - range_T/range_T2*(pR - pL))/2 + pL;
            qDebug() << "m_offset_val_x" << m_offset_val_x;
            update_T();
            update_V();
            DrawResult();
        }
    } else if (event->type() == QEvent::MouseMove)
    {
        if (obj == ui->m_Display) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            int x = mouseEvent->x();
            int y = mouseEvent->y();
            QString Ubuf;
            double U = get_U_from_Y(m_offset_val_y - y);
            if (fabs(U) < 0.1) {
                Ubuf = tr("%1(mV)").arg(QString::number(U*1000.));
            } else {
                Ubuf = tr("%1(V)").arg(QString::number(U));
            }
            QString Tbuf;
            double T = get_T_from_X(x - m_offset_val_x);
            double Tabs = fabs(T);
            if ( Tabs < 0.1e-3) {
                Tbuf = tr("%1(mkSec)").arg(QString::number(T*1000.*1000.));
            } else if (Tabs < 0.1e-3) {
                Tbuf = tr("%1(mSec)").arg(QString::number(T*1000.));
            } else {
                Tbuf = tr("%1(Sec)").arg(QString::number(T));
            }
            QString pos_str;
            pos_str = tr("%1 %2").arg(Ubuf).arg(Tbuf);
            showStatusMessage(pos_str);
        }
    }
    return false;
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::writeData(QString cmd)
{
    m_que_mutex.lock();
    if (m_op_in_process) {
        m_que_mutex.unlock();
        return;
    }
    if (!m_que.isEmpty()) {
        QString cmd_from_que = m_que.at(0);
        qDebug() << "write data"<<cmd_from_que;
        m_op_in_process = true;
        m_op_timer->start(1200);
        m_serial->write(cmd_from_que.toLocal8Bit());
    } else if (cmd.length()>0) {
        m_que.enqueue(cmd);
        qDebug() << "write data"<<cmd;
        m_op_in_process = true;
        m_op_timer->start(1200);
        m_serial->write(cmd.toLocal8Bit());
    }
    m_que_mutex.unlock();
}

void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
    m_responseData += data;

    qDebug() << QString::fromUtf8(m_responseData);

    int ret = checkValidResponse(m_responseData);

    if (ret > 0) {
        showResponse(m_responseData);
        m_responseData.remove(0, ret);
        m_op_timer->stop();
        getcmd(); // extracting cmd from que
    }
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(tr("%1").arg(message));
}

void MainWindow::togleState(bool state_value)
{
    ui->actionConnect->setEnabled(state_value);
    ui->actionDisconnect->setEnabled(!state_value);
    ui->actionRuntimeSettings->setEnabled(!state_value);

    ui->m_runButton->setEnabled(!state_value);
    ui->actionConfigure->setEnabled(state_value);
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        togleState(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        m_que_mutex.lock();
        m_op_in_process = false;
        m_que.clear();
        m_que_mutex.unlock();
        update_delay();
        if (!m_timer->isActive()) {
            on_m_runButton_clicked();
        }
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        showStatusMessage(tr("Open error"));
    }
}
void MainWindow::closeSerialPort()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }

    if (m_serial->isOpen())
        m_serial->close();

    togleState(true);

    m_que_mutex.lock();
    m_que.clear();
    m_op_in_process = false;
    m_que_mutex.unlock();

    showStatusMessage(tr("Disconnected"));
}
void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(ui->actionRuntimeSettings, &QAction::triggered, m_rtsettings, &RuntimeDialog::show);
}
void MainWindow::update_delay()
{
    QString cmd = QString("d%1;").arg(m_T_decimation[m_T_per_mark_index]);
    addcmd(cmd);

}
void MainWindow::addcmd(QString cmd)
{
    m_que_mutex.lock();
    m_que.enqueue(cmd);
    qDebug()<<cmd;
    m_que_mutex.unlock();
}
QString MainWindow::getcmd(void)
{
    QString ret;
    m_que_mutex.lock();
    m_op_in_process = false;
    ret = m_que.dequeue();
    m_que_mutex.unlock();
    return ret;
}
QString MainWindow::fetchcmd(void)
{
    QString cmd;
    m_que_mutex.lock();
    if (!m_que.isEmpty())
        cmd = m_que.at(0);
    m_que_mutex.unlock();
    return cmd;
}
void MainWindow::conver_result(QStringList result)
{
    int i;
    m_result.ts = Line;
    m_result.step_t = (m_T_decimation[m_T_per_mark_index]+1)/Fsamp;
    m_result.vector.clear();
    for (i=2;i<result.count();i++) {
        MyPoint mp;
        memset(&mp, 0, sizeof(mp));
        QString st = result.at(i);
        mp.Y = st.toDouble();
        m_result.vector.append(mp);
    }
    double change_count = result.at(0).toDouble();
    double samples_count = result.at(1).toDouble();
    if (samples_count > 1 ) {
       m_Fr =  change_count/2/samples_count*Fsamp;
    }
}
void MainWindow::showResponse(const QByteArray &data)
{
    QString buf = QString::fromUtf8(data);

    int pos_dollar = buf.indexOf(QChar('$'), 0);
    if (pos_dollar < 0) {
        return;
    }

    int pos = buf.indexOf(QChar('#'), 0);
    if (pos < 0) {
        return;
    }

    QString req = buf.mid(pos_dollar+1, pos - pos_dollar - 1);

    if (pos > 0)
        buf.remove(0, pos);

    pos = buf.indexOf(QChar(':'), pos+1);
    if (pos < 0) {
        return;
    }

    QString num = buf.mid(1, pos - 1);
    if (req.at(0) == QChar('s')) {
        print("s");
        QStringList result = num.split(QChar(';'));
        conver_result(result);
        DrawResult();
    } else if (req.at(0) == QChar('d'))
    {
        print("d");
        pos = num.indexOf(QChar(';'), 0);
        if (pos>=0) {
            print(num.mid(0, pos));
        }
        return;
    } else if (req.at(0) == QChar('t'))
    {
        print("t");
        QStringList result = num.split(QChar(';'));
        conver_result(result);
        DrawResult();
    }
}

int MainWindow::checkValidResponse(const QByteArray &data)
{
    QString buf = QString::fromUtf8(data);

    int pos_dollar = buf.indexOf(QChar('$'), 0);
    if (pos_dollar < 0) {
        return -1;
    }

    int pos1 = buf.indexOf(QChar('#'), pos_dollar + 1);
    if (pos1 < 0) {
        return -1;
    }

    int pos2 = buf.indexOf(QChar(':'), pos1 + 1);

    if (pos2 < 0) {
        return -1;
    }
    return pos2;
}
void MainWindow::op_timeout()
{
    QString cmd;
    m_que_mutex.lock();
    if (!m_que.isEmpty()) {
        m_op_in_process = false;
        cmd = m_que.at(0);
    }
    m_que_mutex.unlock();

    if (cmd.length()>0) {
        writeData(cmd);
    }
}
void MainWindow::transaction()
{
    QString cmd;
    if (m_trigger.tr_type == MainWindow::Positive) {
        cmd = tr("t+%1;").arg(QString::number((int)m_trigger.code));
    } else if (m_trigger.tr_type == MainWindow::Negative) {
        cmd = tr("t-%1;").arg(QString::number((int)m_trigger.code));
    } else {
        cmd = "s";
    }

    writeData(cmd);
}
void MainWindow::DrawResultSelectPen(QPainter *p)
{
    p->setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap));
}
void MainWindow::draw_gnd(QPainter *p)
{
    double x, y;
    QPoint from, to;
    double CodeUin0 = m_rtsettings->psettings()->CodeUin0;
    y = -get_Y_px(CodeUin0) + m_offset_val_y;
    if (y>pB) { y = pB;} if (y<pT) { y = pT; }
    from = QPoint(pL-10, y);
    to = QPoint(pL, y);

    DrawResultSelectPen(p);
    p->drawLine(from, to);
    x = from.x(); y = from.y();
    to = QPoint(x, y + 5);
    p->drawLine(from, to);
    y+=5;
    from = QPoint(x-6, y);
    to = QPoint(x+6, y);
    p->drawLine(from, to);
    y+=3;
    from = QPoint(x-3, y);
    to = QPoint(x+3, y);
    p->drawLine(from, to);
    y+=3;
    p->drawPoint(x, y);
}

void MainWindow::DrawResult(void)
{
    QPicture pi;
    QPainter p;
    p.begin(&pi);
    p.drawPicture(0, 0, *m_pi);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPoint(0, 0), QPoint(pR+20, 0));
    p.drawLine(QPoint(pR+20, 0), QPoint(pR+20, pB+20));
    p.drawLine(QPoint(pR+20, pB+20), QPoint(0, pB+20));
    p.drawLine(QPoint(0, pB+20), QPoint(0, 0));

    p.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap));
    QString num;
    int i;
    MyPoint mp;
    QPoint prev_from, prev_to, from, to;
    double x, min_y, max_y;
    if (m_result.ts == Bar) {
        for (i=0; i<m_result.vector.count(); i++) {
            mp = m_result.vector.at(i);
            if (i != 0)
                num += ";";
            num += QString().number((mp.max_y + mp.min_y)/2);
            if (i > 0){
                x = get_T_px(i) + m_offset_val_x;
                max_y = -get_Y_px(mp.max_y) + m_offset_val_y;
                min_y= -get_Y_px(mp.min_y)  + m_offset_val_y;
                from = QPoint(x, min_y); to = QPoint(x, max_y);
                drawLine(&p, prev_from, from);
                drawLine(&p, prev_to, to);
                drawLine(&p, from, to);
            } else {
                x = get_T_px(i) + m_offset_val_x;
                max_y = -get_Y_px(mp.max_y) + m_offset_val_y;
                min_y = -get_Y_px(mp.min_y)  + m_offset_val_y;
                from = QPoint(x, min_y); to = QPoint(x, max_y);
                drawLine(&p, from, to);
            }
            prev_from = from;
            prev_to = to;
        }
    } else if (m_result.ts == Line) {
        for (i=0; i<m_result.vector.count(); i++) {
            if (i != 0)
                num += ";";
            mp = m_result.vector.at(i);
            num += QString().number(mp.Y);
            if (i > 0) {
                to = QPoint(get_T_px(i) + m_offset_val_x,
                            -get_Y_px(mp.Y)  + m_offset_val_y);
                drawLine(&p, prev_to, to);
            } else {
                to = QPoint(get_T_px(i) + m_offset_val_x,
                            -get_Y_px(mp.Y) + m_offset_val_y);
            }
            prev_to = to;
        }
    }

    if ((m_trigger.tr_type == Positive) || (m_trigger.tr_type == Negative)) {
        p.setPen(QPen(Qt::yellow, 1, Qt::SolidLine, Qt::RoundCap));
        double y = -get_Y_px(m_trigger.code) + m_offset_val_y;
        from = QPoint (pL, y);
        to = QPoint (pR, y);
        drawLine(&p, from, to);
    }
    draw_gnd(&p);
    p.end();
    if (m_result.vector.count() > 0) {
        print(num);
    }
    ui->m_Display->setPicture(pi);
    update_Fr();
}
void MainWindow::draw_and_check_line(QPainter *p, QPoint from, QPoint to)
{
    if (from.y() < pT ) { from.setY(pT); }
    if (from.y() > pB)  { from.setY(pB); }
    if (to.y() < pT)    { to.setY(pT);   }
    if (to.y() > pB)    { to.setY(pB);   }
    if (from.x() < pL ) { from.setX(pL); }
    if (from.x() > pR)  { from.setX(pR); }
    if (to.x() < pL )   { to.setX(pL);   }
    if (to.x() > pR)    { to.setX(pR);   }
    p->drawLine(from, to);
}
void MainWindow::drawLine(QPainter *p, QPoint from, QPoint to)
{
    bool clip = false;

    if ((from.x() < pL) || (from.y() < pT) || (from.y() > pB) || (from.x() > pR)) clip = true;
    if ((to.x() < pL) || (to.y() < pT) || (to.y() > pB) || (to.x() > pR)) clip = true;

    if (!clip) {
        draw_and_check_line(p, from, to);
        return;
    }

    if ((from.x() == to.x()) || (from.y() == to.y())) {
        // вертикальная линия или горизонтальная линия
        if (from.x() < pL) { from.setX(pL); }
        if (from.x() > pR) { from.setX(pR); }
        if (to.x() < pL) { to.setX(pL); }
        if (to.x() > pR) { to.setX(pR); }
        if (from.y() < pT) { from.setY(pT); }
        if (from.y() > pB) { from.setY(pB); }
        if (to.y() < pT) { to.setY(pT); }
        if (to.y() > pB) { to.setY(pB); }
                draw_and_check_line(p, from, to);
        return;
    }

    double x1, x2, y1, y2, dx, dy, yx, xy, ox, oy, x, y, bx, by;
    x1 = from.x(); y1 = from.y(); x2 = to.x(); y2 = to.y(); dx = x2 - x1; dy = y1 - y2;
    yx = dy/dx; xy = dx/dy;

    if (!inside_display(x1, y1, x2, y2)) {
        if (x1 < pL) from.setX(pL);
        if (x1 > pR) from.setX(pR);
        if (y1 < pT) from.setY(pT);
        if (y1 > pB) from.setY(pB);
        if (x2 < pL) to.setX(pL);
        if (x2 > pR) to.setX(pR);
        if (y2 < pT) to.setY(pT);
        if (y2 > pB) to.setY(pB);
        draw_and_check_line(p, from, to);
        return;
    }

    if ((x1 < pL) && (y1 > pB)) {
       x = (pL - x1);
       y = (y1 - pB);
       oy = yx * x;
       ox = xy * y;
       by = ( oy > y ? oy : y );
       bx = ( ox > x ? ox : x );
       draw_and_check_line(p, QPoint(pL, pB), QPoint(x1 + bx, y1 - by));
       from.setX(x1 + bx);
       from.setY(y1 - by);
    } else if ((x1 < pL) && (y1 < pT)) {
       x = (pL - x1);
       y = (pT - y1);
       oy = fabs(yx * x);
       ox = fabs(xy * y);
       by = ( oy > y ? oy : y );
       bx = ( ox > x ? ox : x );
       draw_and_check_line(p, QPoint(pL, pT), QPoint(x1 + bx, y1 + by));
       from.setX(x1 + bx);
       from.setY(y1 + by);
    } else if ((x1 > pR) && (y1 < pT)) {
       x = (x1 - pR);
       y = (pT - y1);
       oy = fabs(yx * x);
       ox = fabs(xy * y);
       by = ( oy > y ? oy : y );
       bx = ( ox > x ? ox : x );
       draw_and_check_line(p, QPoint(pR, pT), QPoint(x1 - bx, y1 + by));
       from.setX(x1 - bx);
       from.setY(y1 + by);
    } else if ((x1 > pR) && (y1 > pB)) {
        x = (x1 - pR);
        y = (y1 - pB);
        oy = fabs(yx * x);
        ox = fabs(xy * y);
        by = ( oy > y ? oy : y );
        bx = ( ox > x ? ox : x );
        draw_and_check_line(p, QPoint(pR, pB), QPoint(x1 - bx, y1 - by));
        from.setX(x1 - bx);
        from.setY(y1 - by);
    } else if (x1 < pL) {
        x = (pL - x1);
        oy = yx * x;
        draw_and_check_line(p, QPoint(pL, y1), QPoint(pL, y1 - oy));
        from.setX(pL);
        from.setY(y1 - oy);
    } else if (x1 > pR) {
        x = (x1 - pR);
        oy = yx * x;
        draw_and_check_line(p, QPoint(pR, y1), QPoint(pR, y1 + oy));
        from.setX(pR);
        from.setY(y1 + oy);
    }  else if (y1 > pB) {
        y = (y1 - pB);
        ox = xy * y;
        draw_and_check_line(p, QPoint(x1, pB), QPoint(x1 + ox, pB));
        from.setX(x1 + ox);
        from.setY(pB);
    } else if (y1 < pT) {
        y = (pT - y1);
        ox = xy * y;
        draw_and_check_line(p, QPoint(x1, pT), QPoint(x1 - ox, pT));
        from.setX(x1 - ox);
        from.setY(pT);
    }

    if ((x2 < pL) && (y2 > pB)) {
       x = (pL - x2);
       y = (y2 - pB);
       oy = yx * x;
       ox = xy * y;
       by = ( oy > y ? oy : y );
       bx = ( ox > x ? ox : x );
       draw_and_check_line(p, QPoint(pL, pB), QPoint(x2 + bx, y2 - by));
       to.setX(x2 + bx);
       to.setY(y2 - by);
    } else if ((x2 < pL) && (y2 < pT)) {
       x = (pL - x2);
       y = (pT - y2);
       oy = fabs(yx * x);
       ox = fabs(xy * y);
       by = ( oy > y ? oy : y );
       bx = ( ox > x ? ox : x );
       draw_and_check_line(p, QPoint(pL, pT), QPoint(x2 + bx, y2 + by));
       to.setX(x2 + bx);
       to.setY(y2 + by);
    } else if ((x2 > pR) && (y2 < pT)) {
        x = (x2 - pR);
        y = (pT - y2);
        oy = fabs(yx * x);
        ox = fabs(xy * y);
        by = ( oy > y ? oy : y );
        bx = ( ox > x ? ox : x );
        draw_and_check_line(p, QPoint(pR, pT), QPoint(x2 - bx, y2 + by));
        to.setX(x2 - bx);
        to.setY(y2 + by);
    } else if ((x2 > pR) && (y2 > pB)) {
        x = (x2 - pR);
        y = (y2 - pB);
        oy = fabs(yx * x);
        ox = fabs(xy * y);
        by = ( oy > y ? oy : y );
        bx = ( ox > x ? ox : x );
        draw_and_check_line(p, QPoint(pR, pB), QPoint(x2 - bx, y2 - by));
        to.setX(x2 - bx);
        to.setY(y2 - by);
    } else if (x2 < pL) {
        x = (pL - x2);
        oy = yx * x;
        draw_and_check_line(p, QPoint(pL, y2), QPoint(pL, y2 - oy));
        to.setX(pL);
        to.setY(y2 - oy);
    } else if (x2 > pR) {
        x = (x2 - pR);
        oy = yx * x;
        draw_and_check_line(p, QPoint(pR, y2), QPoint(pR, y2 + oy));
        to.setX(pR);
        to.setY(y2 + oy);
    }  else if (y2 > pB) {
        y = (y2 - pB);
        ox = xy * y;
        draw_and_check_line(p, QPoint(x2, pB), QPoint(x2 + ox, pB));
        to.setX(x2 + ox);
        to.setY(pB);
    } else if (y2 < pT) {
        y = (pT - y2);
        ox = xy * y;
        draw_and_check_line(p, QPoint(x2, pT), QPoint(x2 - ox, pT));
        to.setX(x2 - ox);
        to.setY(pT);
    }

    draw_and_check_line(p,from, to);
    return;
}
double MainWindow::get_T_sec(long index_t)
{
    return index_t * m_result.step_t;
}
double MainWindow::get_T_px(long index_t)
{
    double pixel_per_mark_X = ((double)(pR - pL)) / nX; // 20 пикселей на одно деление
    double T_sec = get_T_sec(index_t);
    return pixel_per_mark_X * T_sec/m_T_per_mark[m_T_per_mark_index];
}
double MainWindow::get_T_from_X(long X)
{
    double pixel_per_mark_X = ((double)(pR - pL)) / nX; // 20 пикселей на одно деление
    double T_sec = m_T_per_mark[m_T_per_mark_index] * X/pixel_per_mark_X;
    return T_sec;
}
double MainWindow::get_Y_px(double code)
{
    double pixel_per_mark_Y = ((double)(pB - pT)) / nY; // 20 пикселей на одно деление
    double U_val = get_U_from_code(code);
    return pixel_per_mark_Y*U_val/m_V_per_mark[m_V_per_mark_index];
}
double MainWindow::get_U_from_code(double code)
{
    RuntimeDialog::Settings *st = m_rtsettings->psettings();
    const double R1 = st->R1 + 100; // 100 == Rcable
    const double R2 = st->R2;
    const double R3 = st->R3;
    const double Range = 1024.0;
    const double Uref = st->Uref;
    const double U3 = (code/Range)*Uref;
    const double U_val = (U3*(1+(R3/R1+R3/R2))-Uref*R3/R2)*R1/R3;

    return U_val;
}
double MainWindow::get_code_from_U(double Uin)
{
    RuntimeDialog::Settings *st = m_rtsettings->psettings();
    const double R1 = st->R1 + 100; // 100 == Rcable
    const double R2 = st->R2;
    const double R3 = st->R3;
    const double Range = 1024.0;
    const double Uref = st->Uref;
    const double U_val =(Uin*R3/R1+Uref*R3/R2)/(1+R3/R1+R3/R2);
    const double code = U_val*Range/Uref;
    return code;
}
double MainWindow::get_U_from_Y(double Y)
{
    double pixel_per_mark_Y = ((double)(pB - pT)) / nY; // 20 пикселей на одно деление
    double U_val = Y/pixel_per_mark_Y*m_V_per_mark[m_V_per_mark_index];;
    return U_val;
}
bool MainWindow::inside_display(double x1, double y1, double x2, double y2)
{
    if ((x1 < pL) && (x2 < pL)) {
        return false;
    }
    if ((x1 > pR) && (x2 > pR)) {
        return false;
    }
    if ((y1 < pT) && (y2 < pT)) {
        return false;
    }
    if ((y1 > pB) && (y2 > pB)) {
        return false;
    }
    return true;
}
void MainWindow::slotCustomMenuRequested(QPoint pos)
{
    m_cnt_menu_point = pos;
    /* Вызываем контекстное меню */
    m_cnt_menu->popup(ui->m_Display->mapToGlobal(pos));
}
void MainWindow::slotPositiveTrigger(bool checked)
{
    (void)(checked);
    ui->actionNegative->setChecked(false);
    ui->actionPositive->setChecked(true);
    ui->actionNo_trigger->setChecked(false);
    double U = get_U_from_Y(m_offset_val_y - m_cnt_menu_point.y());
    m_trigger.tr_type = Positive;
    m_trigger.code = get_code_from_U(U);
    m_result.vector.clear();
    DrawResult();
}
void MainWindow::slotNegativeTrigger(bool checked)
{
    (void)(checked);
    ui->actionNegative->setChecked(true);
    ui->actionPositive->setChecked(false);
    ui->actionNo_trigger->setChecked(false);
    double U = get_U_from_Y(m_offset_val_y - m_cnt_menu_point.y());
    m_trigger.tr_type = Negative;
    m_trigger.code = get_code_from_U(U);
    m_result.vector.clear();
    DrawResult();
}
void MainWindow::slotFreeRunningTrigger(bool checked)
{
    (void)(checked);
    ui->actionNegative->setChecked(false);
    ui->actionPositive->setChecked(false);
    ui->actionNo_trigger->setChecked(true);
    m_trigger.tr_type = FreeRunning;
    m_trigger.code = get_code_from_U(0);
    DrawResult();
}

