#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settingsdialog.h"

#include <QMainWindow>
#include <QList>
#include <QTextStream>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QScrollBar>
#include <QMessageBox>
#include <QDir>
#include <QTextBlock>
#include <QTableWidgetItem>
#include <QSerialPort>
#include <QTime>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class SettingsDialog;
class Console;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString timeStamp();


public slots:

    void hexHyperTermEntered();
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void loadCommandMacro();
    void toolbarMapping();
    void updateMacroListFile();
    void addCommandMacro();
    void deleteCommandMacro();
    void createBatchCommand();
    void loadMacroListFile();
    void cleanLogScreen();
    void lightOn(QString);
    void close();
    void doRemainingDirtyInitialization();
    void handleMacroCommandRun(int);
    void verifyResponse();
    void writeHexData();
    void writeAsciiData();

private:
    Ui::MainWindow *ui;

    int transactionCount;
    void initActionsConnections();
    SettingsDialog *settings;
    Console* console;
    QSerialPort *serial;
    QTimer timer;
    QByteArray data;
    QString responseCatcher;
    bool localEchoFlag;
    void cleanFields();

};

#endif // MAINWINDOW_H
