#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QTimer>

constexpr double batt_temp_error_min = 60.0;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  showFullScreen();

  QTimer::singleShot(100, this, SLOT(fix_ui()));

  /// arduino communication
  connect(&arduino, SIGNAL(msg_complete(Msg)), this, SLOT(msg_recvd(Msg)));
  thr = new QThread();
  arduino.moveToThread(thr);
  connect(thr, SIGNAL(started()), &arduino, SLOT(wait_read()));
  thr->start();
}

void MainWindow::fix_ui(){
  /// LOGOS
  QPixmap boun_logo(":/new/logos/boun_logo.png");
  QPixmap mech_turk(":/new/logos/mech_turk.png");
  ui->mech_turk_label->setPixmap(mech_turk.scaled(ui->mech_turk_label->width(), ui->mech_turk_label->height(), Qt::KeepAspectRatio));
  ui->boun_label->setPixmap(boun_logo.scaled(ui->boun_label->width(), ui->boun_label->height(), Qt::KeepAspectRatio));

  /// battery logo
  battery_logo = QPixmap(":/new/logos/pil.png");
  change_battery_logo(100);

  /// battery temp warning logo
  batt_warning = QPixmap(":/new/logos/batt_temp.png");

  /// km/h font scaling
  QFont font = ui->kmh_label->font();
  QRect textrect = QFontMetrics(font).boundingRect("km/h");
  QRect availablerect = ui->kmh_label->contentsRect();
  float factor = availablerect.width() / (float)textrect.width();
  font.setPointSize(font.pointSize() * factor);
  ui->kmh_label->setFont(font);

  /// volt_1_label font scaling
  textrect = QFontMetrics(font).boundingRect("Voltage 1: 44.44V");
  availablerect = ui->volt_1_label->contentsRect();
  factor = availablerect.width() / (float)textrect.width();
  font.setPointSize(font.pointSize() * factor);
  ui->volt_1_label->setFont(font);

  /// volt_2_label font scaling
  textrect = QFontMetrics(font).boundingRect("Voltage 2: 44.44V");
  availablerect = ui->volt_2_label->contentsRect();
  factor = availablerect.width() / (float)textrect.width();
  font.setPointSize(font.pointSize() * factor);
  ui->volt_2_label->setFont(font);

  /// curr_label font scaling
  textrect = QFontMetrics(font).boundingRect("Batt Amps: 44.44A");
  availablerect = ui->curr_label->contentsRect();
  factor = availablerect.width() / (float)textrect.width();
  font.setPointSize(font.pointSize() * factor);
  ui->curr_label->setFont(font);

  /// batt_temp_label font scaling
  textrect = QFontMetrics(font).boundingRect("Batt Temp: 44.44°C");
  availablerect = ui->batt_temp_label->contentsRect();
  factor = availablerect.width() / (float)textrect.width();
  font.setPointSize(font.pointSize() * factor);
  ui->batt_temp_label->setFont(font);

  /// batt_volt font scaling
  textrect = QFontMetrics(font).boundingRect("Batt Volt: 44.44V");
  availablerect = ui->batt_volt_label->contentsRect();
  factor = availablerect.width() / (float)textrect.width();
  font.setPointSize(font.pointSize() * factor);
  ui->batt_volt_label->setFont(font);

  QTimer::singleShot(100, this, SLOT(send_req()));
}

uint8_t last_req_idx = 0;
bool req_ans_came = false;
void MainWindow::send_req(){
  if (req_ans_came) last_req_idx++;
  if (last_req_idx > 5) last_req_idx = 0;
  arduino.send_only_cmd(cmds[last_req_idx]);

  QTimer::singleShot(10, this, SLOT(send_req()));
}

MainWindow::~MainWindow()
{
  delete ui;
  delete thr;
}

uint8_t get_battery_percentage(double voltage){
  constexpr double max_volt = 67.2;
  constexpr double min_volt = 56.0;

  if (voltage < min_volt) return 0;
  if (voltage > max_volt) return 100;
  return 100 * (voltage - min_volt) / (max_volt - min_volt);
}
void MainWindow::change_battery_logo(uint8_t perc){
  static uint8_t last_perc = -1;
  if (perc == last_perc) return;
  last_perc = perc;

  QImage battery = QImage(battery_logo.width(), battery_logo.height(), QImage::Format_ARGB32);

  QPainter p(&battery);
  // p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
  if (perc < 20)
    p.setBrush(QBrush(Qt::red));
  else if (perc < 70)
    p.setBrush(QBrush(Qt::yellow));
  else
    p.setBrush(QBrush(Qt::green));
  QPen pen = p.pen();
  pen.setWidth(0);
  p.setPen(pen);
  p.drawRect(20, 20, (int)(297 * perc / 100.0), 156);
  p.drawPixmap(0, 0, battery_logo);

  ui->battery_label->setPixmap(QPixmap::fromImage(battery).scaled(ui->battery_label->width(), ui->battery_label->height(), Qt::KeepAspectRatio));
}

void MainWindow::msg_recvd(Msg msg){
  req_ans_came = true;

  uint8_t cmd = arduino.get_cmd(msg);
  if (cmd == batt_temp_cmd){
    uint16_t batt_temp_i = arduino.get_uint16(msg, 0);
    double batt_temp = batt_temp_i / 100.0;

    if (batt_temp >= batt_temp_error_min)
      ui->batt_warning_label->setPixmap(batt_warning.scaled(ui->batt_warning_label->width(), ui->batt_warning_label->height(), Qt::KeepAspectRatio));
    else ui->batt_warning_label->clear();

    ui->batt_temp_label->setText(QString("Batt Temp: %1°C").arg(batt_temp, 0, 'f', 2));
  }
  else if (cmd == batt_volt_cmd){
    uint16_t batt_volt_i = arduino.get_uint32(msg, 0);
    double batt_volt = batt_volt_i / 100.0;

    change_battery_logo(get_battery_percentage(batt_volt));
    ui->batt_volt_label->setText(QString("Batt Volt: %1V").arg(batt_volt, 0, 'f', 2));
  }
  else if (cmd == volt1_cmd){
    uint16_t volt_i = arduino.get_uint32(msg, 0);
    double volt = volt_i / 100.0;

    ui->volt_1_label->setText(QString("Voltage 1: %1V").arg(volt, 0, 'f', 2));
  }
  else if (cmd == volt2_cmd){
    uint16_t volt_i = arduino.get_uint32(msg, 0);
    double volt = volt_i / 100.0;

    ui->volt_2_label->setText(QString("Voltage 2: %1V").arg(volt, 0, 'f', 2));
  }
  else if (cmd == tuketim_cmd){
    uint16_t curr_i = arduino.get_uint32(msg, 0);
    double curr = curr_i / 100.0;

    ui->curr_label->setText(QString("Batt Amps: %1A").arg(curr, 0, 'f', 2));
  }
  else if (cmd == speed_cmd){
    uint16_t speed_i = arduino.get_uint16(msg, 0);
    double speed = speed_i / 10.0;

    ui->speed_label->display(QString("%1").arg(speed, 0, 'f', 1));
  }
}
