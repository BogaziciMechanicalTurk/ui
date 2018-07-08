#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QThread>
#include "arduino.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  Arduino arduino;
  QThread *thr;

  QPixmap battery_logo;
  QPixmap batt_warning;

  void change_battery_logo(uint8_t perc);

public slots:
  void msg_recvd(Msg msg);
  void fix_ui();
  void send_req();
};

#endif // MAINWINDOW_HPP
