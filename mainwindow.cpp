#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // load macro list
    loadMacroListFile();

    serial = new QSerialPort(this);
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    toolbarMapping();    

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));

    initActionsConnections();
    doRemainingDirtyInitialization();

    // Fit in the window frame
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
}

MainWindow::~MainWindow()
{
    updateMacroListFile();
    delete settings;
    delete ui;
}

void
MainWindow::
initActionsConnections()
{
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)), SLOT(loadCommandMacro()));
    connect(ui->actionClean_Log_Screen, SIGNAL(triggered()), this, SLOT(cleanLogScreen()));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addCommandMacro()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteCommandMacro()));
    connect(ui->selectMacro, SIGNAL(currentIndexChanged(int)), this, SLOT(handleMacroCommandRun(int)));
    connect(ui->tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(doRemainingDirtyInitialization()));
    connect(ui->request, SIGNAL(returnPressed()), ui->sendButton, SLOT(click()));
    connect(ui->hyperTerm, SIGNAL(returnPressed()), this, SLOT(hexHyperTermEntered()));
    connect(ui->hyperTerm2, SIGNAL(returnPressed()), this, SLOT(writeAsciiData()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(verifyResponse()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(writeHexData()));
}


void
MainWindow::
toolbarMapping()
{
    ui->mainToolBar->addAction(ui->actionConfigure);
    ui->mainToolBar->addAction(ui->actionConnect);
    ui->mainToolBar->addAction(ui->actionDisconnect);
    ui->mainToolBar->addAction(ui->actionClean_Log_Screen);
}


void
MainWindow::
doRemainingDirtyInitialization()
{
    ui->selectMacro->clear();

    ui->tempEdit->hide(); // temp board
    if (ui->tableWidget->rowCount() > 0)
    {
        for ( int i = 0; i < ui->tableWidget->rowCount(); i++)
            ui->selectMacro->addItem(ui->tableWidget->item(i,0)->text());
    }
    //ui->request->clear();
    ui->delay->clear();
}


void
MainWindow::
hexHyperTermEntered()
{
    if ( !ui->hyperTerm->text().isEmpty() )
    {
        ui->request->clear();
        ui->request->setText(ui->hyperTerm->text());
        writeHexData();
        ui->hyperTerm->clear();
    }
}


void
MainWindow::
handleMacroCommandRun(int row)
{
    cleanFields();
    ui->request->setText( ui->tableWidget->item(row,1)->text() );
    ui->delay->setText( ui->tableWidget->item(row,2)->text() );
    if ( ui->tableWidget->item(row,3)->text().contains("N/A") )
        ui->response->clear();
    else
        ui->response->setText( ui->tableWidget->item(row,3)->text() );
}


void
MainWindow::
openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    localEchoFlag = p.localEchoEnabled;
    serial->setPortName(p.name);
    if (serial->open(QIODevice::ReadWrite))
    {
        if (serial->setBaudRate(p.baudRate) &&
            serial->setDataBits(p.dataBits) &&
            serial->setParity(p.parity) &&
            serial->setStopBits(p.stopBits) &&
            serial->setFlowControl(p.flowControl))
        {
            ui->plainTextEdit->setStyleSheet("color: white;" "background-color: black;" "selection-color: yellow;" "selection-background-color: blue;");
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
            ui->actionConfigure->setEnabled(false);
            ui->sendButton->setEnabled(true);
            ui->selectMacro->setEnabled(true);
            ui->selectBatch->setEnabled(true);
            ui->addButton->setEnabled(false);
            ui->deleteButton->setEnabled(false);
            ui->hyperTerm->setEnabled(true);
            ui->hyperTerm2->setEnabled(true);
            ui->request->setEnabled(true);
            ui->response->setEnabled(true);
            ui->delay->setEnabled(true);
            ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

        }
        else
        {
            serial->close();
            QMessageBox::critical(this, tr("Error"), serial->errorString());

            ui->statusBar->showMessage(tr("Open error"));
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        ui->statusBar->showMessage(tr("Configure error"));
    }
}


void
MainWindow::
closeSerialPort()
{
    serial->close();
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    ui->sendButton->setEnabled(false);
    ui->request->clear();
    ui->response->clear();
    ui->request->setEnabled(false);
    ui->response->setEnabled(true);
    ui->addButton->setEnabled(true);
    ui->selectMacro->setEnabled(false);
    ui->selectBatch->setEnabled(false);
    ui->hyperTerm->setEnabled(false);
    ui->hyperTerm2->setEnabled(false);
    ui->deleteButton->setEnabled(true);
    ui->addButton->setEnabled(true);
    ui->statusBar->showMessage(tr("Disconnected"));
    ui->plainTextEdit->clear();
    ui->hyperTerm->clear();
    ui->hyperTerm2->clear();
    ui->delay->setEnabled(false);
    ui->plainTextEdit->setStyleSheet("color: black;" "background-color: white;");
}


void
MainWindow::
writeHexData()
{
    if (!ui->request->text().isEmpty())
    {
        QByteArray dataArray = ui->request->text().toLocal8Bit();
        ui->plainTextEdit->insertPlainText(ui->request->text());
        if (localEchoFlag)
        {
            serial->write(QByteArray::fromHex( dataArray.append("\r\n")));
        }
        else
            serial->write(QByteArray::fromHex( ui->request->text().toLocal8Bit()));

        ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->maximum());
        ui->tabWidget->setCurrentIndex(0);
        if (!ui->delay->text().isEmpty())
            timer.start(ui->delay->text().toInt());
    }
    else
        return;
}

void
MainWindow::
writeAsciiData()
{
    if (!ui->hyperTerm2->text().isEmpty())
    {
        QByteArray dataArray = ui->hyperTerm2->text().toLocal8Bit();
        ui->plainTextEdit->insertPlainText(ui->hyperTerm2->text());
        if (localEchoFlag)
        {
            serial->write(dataArray.append("\r\n"));
        }
        else
            serial->write(dataArray);

        ui->hyperTerm2->clear();
        ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->maximum());
        ui->tabWidget->setCurrentIndex(0);
        if (!ui->delay->text().isEmpty())
            timer.start(ui->delay->text().toInt());
    }
    else
        return;
}


void
MainWindow::
readData()
{
    QByteArray dataArray = serial->readAll();
    ui->plainTextEdit->insertPlainText(QString(dataArray));
    if (!ui->response->text().isEmpty())
    {
        QString string_a(dataArray);
        responseCatcher.append(string_a);
    }
    ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->maximum());
}


QString
MainWindow::
timeStamp()
{
    return QString(QTime::currentTime().toString().prepend("[ ").append(" ] "));
}


void
MainWindow::
handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}


void
MainWindow::
close()
{
    QApplication::quit();
}

void
MainWindow::
addCommandMacro()
{
    QItemSelectionModel *select = ui->tableWidget->selectionModel();

    select->currentIndex().row() > -1 ? ui->tableWidget->insertRow(select->currentIndex().row()+1) :
                                        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
}

void
MainWindow::
deleteCommandMacro()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Sparky"
                                                              ,"Are you sure you want to delete?"
                                                              ,QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        QItemSelectionModel *select = ui->tableWidget->selectionModel();
        ui->tableWidget->removeRow(select->currentIndex().row());
    }
}


void
MainWindow::
loadCommandMacro()
{
    if (ui->request->isEnabled())
    {
        QItemSelectionModel *select = ui->tableWidget->selectionModel();

        int row = select->currentIndex().row();

        if (!ui->tableWidget->item(row,0)->text().isEmpty() &&
            !ui->tableWidget->item(row,1)->text().isEmpty() &&
            !ui->tableWidget->item(row,2)->text().isEmpty() )
        {
            ui->request->setText(ui->tableWidget->item(row,1)->text());
            ui->delay->setText(ui->tableWidget->item(row,2)->text());
            ui->selectMacro->setCurrentIndex(row);
            if ( !ui->tableWidget->item(row,3)->text().contains("N/A") )
                ui->response->setText(ui->tableWidget->item(row,3)->text());
            else
                ui->response->clear();
            lightOn("standby");
        }
    }
}


void
MainWindow::
updateMacroListFile()
{
    QFile file(QCoreApplication::applicationDirPath()+"/"+"macro_from_Trane_Sparky.spk");
    QTextStream out(&file);

    if (!file.open(QIODevice::WriteOnly) || ui->tableWidget->rowCount() == 0)
        return;
    else
    {
        QString itemData;
        for ( int j = 0; j < ui->tableWidget->rowCount(); j++ )
        {
            QTableWidgetItem* item0 = ui->tableWidget->item(j,0);
            QTableWidgetItem* item1 = ui->tableWidget->item(j,1);
            QTableWidgetItem* item2 = ui->tableWidget->item(j,2);
            QTableWidgetItem* item3 = ui->tableWidget->item(j,3);

            if ( item0 && item1 && item2 && item3 &&
                 !item0->text().isEmpty() && !item1->text().isEmpty() &&
                 !item2->text().isEmpty() && !item3->text().isEmpty())
            {
                for ( int k = 0; k < ui->tableWidget->columnCount(); k++ )
                {
                    QTableWidgetItem* item = ui->tableWidget->item(j,k);
                    itemData.append(item->text()).append(" | ");
                }
                out << itemData.append("\n");
                itemData.clear();
            }
        }

        file.close();
    }
}



void
MainWindow::
loadMacroListFile()
{
    QFile file(QCoreApplication::applicationDirPath()+"/"+"macro_from_Trane_Sparky.spk");
    if (!file.open(QIODevice::ReadOnly) || file.size() == 0)
       return;
    else
    {
        QTextStream textStream(&file);
        ui->tempEdit->setPlainText(textStream.readAll().trimmed());
        for ( int row = 0; row < ui->tempEdit->document()->lineCount(); row++ )
        {
            QString line = ui->tempEdit->document()->findBlockByLineNumber(row).text();
            ui->tableWidget->insertRow(row); // add a row to the table
            for ( int col = 0; col < ui->tableWidget->columnCount(); col++ )
            {
               QTableWidgetItem * item = new QTableWidgetItem(line.split("|").at(col).trimmed());
               ui->tableWidget->setItem(row,col,item);
            }
        }
    }
}


void
MainWindow::
verifyResponse()
{
    if ( !ui->response->text().isEmpty() )
    {
        if (responseCatcher.contains(ui->response->text()))
            ui->response->setText("PASS");
        else
            ui->response->setText("FAIL");

        responseCatcher.clear();
    }
}

void
MainWindow::
createBatchCommand()
{

}


void
MainWindow::
lightOn(QString b)
{
    if ( b == "true" )
    {
       ui->response->setStyleSheet("Qrequest{background: green;}");
    }
    else if ( b == "false" )
    {
       ui->response->setStyleSheet("Qrequest{background: red;}");
    }
    else
    {
       ui->response->setStyleSheet("Qrequest{background: white;}");
    }
}


void
MainWindow::
cleanLogScreen()
{
    ui->plainTextEdit->clear();
    cleanFields();
    ui->delay->clear();
    ui->response->clear();
}

void
MainWindow::
cleanFields()
{
    ui->hyperTerm->clear();
    ui->hyperTerm2->clear();
    ui->request->clear();
}
