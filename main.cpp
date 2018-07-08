#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
  qRegisterMetaType<Msg>("Msg");

  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
