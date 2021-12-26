#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QBasicTimer>
#include <QSerialPort>
#include <QLabel>
#include <QQueue>
#include <QMutex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SettingsDialog;
class RuntimeDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum MyTypeSelector { Undefined, Line, Bar };
    enum MyTypeTrigger { FreeRunning, Positive, Negative };

    struct MyPoint {
        double max_y;
        double min_y;
        double Y;
    };

    struct MyResult {
        MyTypeSelector ts;
        double step_t; // in sec
        QVector<MyPoint> vector;
    };

    struct MyTrigger {
        MyTypeTrigger tr_type;
        unsigned long code;
    };

private:
    void initActionsConnections();
    int checkValidResponse(const QByteArray &data);
    void readSettings();
    void writeSettings();
    void writeData(QString cmd = "");

public:
    void handleError(QSerialPort::SerialPortError error);
    void print(const QString &s);

    bool eventFilter(QObject *obj, QEvent *event);

private:
    void DrawResult(void);
    void DrawGrid(QPicture *pi);
    void drawLine(QPainter *p, QPoint from, QPoint to);
    void draw_and_check_line(QPainter *p, QPoint from, QPoint to);
    void DrawResultSelectPen(QPainter *p);
    void draw_gnd(QPainter *p);
    bool trimXY(QPoint &pos);
    void showResponse(const QByteArray &data);
    void addcmd(QString cmd);
    QString getcmd(void);
    QString fetchcmd(void);

private slots:
    void transaction(void);
    void op_timeout(void);
    void processError(const QString &s);
    void processTimeout(const QString &s);
    void on_m_runButton_clicked(void);
    void openSerialPort(void);
    void closeSerialPort(void);
    void readData(void);
    void slotCustomMenuRequested(QPoint pos);
    void slotPositiveTrigger(bool checked = false);
    void slotNegativeTrigger(bool checked = false);
    void slotFreeRunningTrigger(bool checked = false);

public slots:
    void update_delay();

private:
    void showStatusMessage(const QString &message);
    void togleState(bool state_value);
    double getNewOfsetY(int new_index);
    double getNewOfsetX(int new_index);
    double get_T_px(long index_t);
    double get_T_sec(long index_t);
    double get_T_from_X(long X);
    double get_Y_px(double code);
    double get_U_from_code(double code);
    double get_U_from_Y(double Y);
    double get_code_from_U(double U);
    bool inside_display(double x1, double y1, double x2, double y2);
    void conver_result(QStringList result);
    void update_V(void);
    void update_T(void);
    void update_Fr(void);

private:
    Ui::MainWindow *ui;

    QTimer *m_timer;
    QPicture *m_pi;

    const double m_V_per_mark[9] = { 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 5.0 };
    int m_V_per_mark_index = 6;

    const double m_T_per_mark[13] = { 1e-5, 2e-5, 5e-5, 1e-4, 2e-4, 5e-4, 1e-3, 2e-3, 5e-3, 0.01, 0.02, 0.05, 0.1};
    int m_T_per_mark_index = 0;
    const double Fsamp = 16000000.0/16/13;
    unsigned long m_T_decimation[sizeof(m_T_per_mark)/sizeof(m_T_per_mark[0])];

    const QString m_settings_file_name = "application";

    const int pL = 20;
    const int pR = 420;
    const int pT = 20;
    const int pB = 420;
    const int nX = 10;
    const int nY = 10;

    double m_offset_val_y = (pB - pT)/2 + pT;
    double m_offset_val_x = pB;

    QLabel *m_status = nullptr;
    QSerialPort *m_serial = nullptr;
    SettingsDialog *m_settings = nullptr;
    RuntimeDialog *m_rtsettings = nullptr;

    QMutex m_que_mutex;
    QTimer *m_op_timer = nullptr;
    bool m_op_in_process = false;
    QQueue<QString> m_que;
    QByteArray m_responseData;

    MyResult m_result;
    double m_Fr = 0;

    QPoint m_cnt_menu_point = QPoint(0, 0);
    QMenu *m_cnt_menu = nullptr;
    struct MyTrigger m_trigger;

    bool m_control_T_flag = false;
};
#endif // MAINWINDOW_H
