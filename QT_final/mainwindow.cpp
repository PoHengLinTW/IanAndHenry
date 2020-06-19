#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include "sockServer.h"
#include <chrono>
#include <ctime>
#include <QTimer>
#include <iostream>
#include <thread>

void thread_function(){
    sockServer *sS = new sockServer();
    sS->run();
}

static std::string date[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static std::string month[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Nov", "Dec"};
static std::string client_state[3] = {"stopped", "running", "suspended"};
static bool debug = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    flag = false;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    ui->setupUi(this);
    timer->start(1000);
    std::thread t(&thread_function);
    t.detach();
}

MainWindow::~MainWindow()
{
    sockServerStop = true;
    delete ui;
}

void MainWindow::update() {
    if (debug)
        std::cout << ui->time->text().toUtf8().constData() << std::endl;
    if (debug) {
        std::cout << "database: " << database_connection << std::endl;
        std::cout << "client: " << client_connection << std::endl;
        std::cout << "crawling: " << crawling << std::endl;
        std::cout << "report: " << report << std::endl;
    }
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    setTime(now);
    setProgress();
    setWebsite();
    if (flag) {
        setServerInfo(now);
        setClientInfo(now);
    }
    setWorkload();
}

/** Private set function */
void MainWindow::setTime(std::tm* now) {
    //std::string text = date[now->tm_wday] + ", " + month[now->tm_mon] + " " + std::to_string(now->tm_mday) + ", " + std::to_string(now->tm_year+1900);
    ui->date->setText(date2qstring(now));
    //text = std::to_string(now->tm_hour + 1) + " : " + std::to_string(now->tm_min + 1) + " : " + std::to_string(now->tm_sec + 1);
    ui->time->setText(time2qstring(now));
}

void MainWindow::setProgress() {
    if (report) {
        ui->Report->setStyleSheet("QLabel { color: green; }");
        ui->progressBar->setValue(100);
    }
    else if (crawling) {
        ui->Crawling->setStyleSheet("QLabel { color: green; }");
        ui->progressBar->setValue(75);
    }
    else if (client_connection) {
        ui->ClientConnection->setStyleSheet("QLabel { color: green; }");
        ui->progressBar->setValue(50);
        flag = true;
        crawling = true;
    }
    else if (database_connection) {
        ui->DatabaseConnection->setStyleSheet("QLabel { color: green; }");
        ui->progressBar->setValue(25);
    }
}

void MainWindow::setServerInfo(std::tm* now) {
    if (!flag) {
        begin_time = *now;
    }
    double diff = difftime(mktime(now), mktime(&begin_time));
    ui->run_time_val->setText(QString::number(diff));
    ui->begin_time_val->setText(time2qstring(&begin_time));
    ui->speed_val->setText(QString::number(total_count*60/diff));
    ui->total_cnt_val->setText(QString::number(total_count));
    ui->failure_cnt_val->setText(QString::number(failure_count));
    if (total_count==0)
        ui->failure_rate_val->setText(QString::number(0));
    else
        ui->failure_rate_val->setText(QString::number((double)failure_count/total_count));
}

void MainWindow::setWorkload() {
    ui->clientA_val->setText(QString::fromStdString(clientA_name));
    ui->clientB_val->setText(QString::fromStdString(clientB_name));
    ui->clientC_val->setText(QString::fromStdString(clientC_name));

    ui->clientA_state->setValue(clientCrawl[0]*100 / total_B);
    ui->clientB_state->setValue(clientCrawl[1]*100 / total_B);
    ui->clientC_state->setValue(clientCrawl[2]*100 / total_C);

}

void MainWindow::setWebsite() {
    ui->yahoo_crawled_count->setText(QString::number(yahoo_cnt));
    ui->yahoo_failure_count->setText(QString::number(yahoo_fail));
    if ((yahoo_cnt+yahoo_fail)==0)
        ui->yahoo_failure_rate->setText(QString::number(0));
    else
        ui->yahoo_failure_rate->setText(QString::number((double)yahoo_fail/(yahoo_fail+yahoo_cnt)));

    ui->ptt_crawled_count->setText(QString::number(ptt_cnt));
    ui->ptt_failed_count->setText(QString::number(ptt_fail));
    if ((ptt_cnt+ptt_fail)==0)
        ui->ptt_failure_rate->setText(QString::number(0));
    else
        ui->ptt_failure_rate->setText(QString::number((double)ptt_fail/(ptt_cnt+ptt_fail)));

    ui->ettoday_crawled_count->setText(QString::number(ettoday_cnt));
    ui->ettoday_failure_count->setText(QString::number(ettoday_fail));
    if ((ptt_cnt+ptt_fail)==0)
        ui->ettoday_failure_rate->setText(QString::number(0));
    else
        ui->ettoday_failure_rate->setText(QString::number((double)ettoday_fail/(ettoday_cnt+ettoday_fail)));
}

void MainWindow::setClientInfo(std::tm* now) {
    double diff = difftime(mktime(now), mktime(&clientBegin[0]));
    ui->client1_name->setText(QString::fromStdString(clientA_name));
    ui->client1_state->setText(QString::fromStdString(client_state[clientState[0]]));
    ui->client1_crawled_count->setText(QString::number(clientCrawl[0]));
    ui->client1_failure_count->setText(QString::number(clientFail[0]));
    if (clientState[0] == 1) {
        ui->client1_begin_time->setText(time2qstring(&clientBegin[0]));
        ui->client1_crawled_time->setText(QString::number(diff));
    }

    diff = difftime(mktime(now), mktime(&clientBegin[1]));
    ui->client2_name->setText(QString::fromStdString(clientB_name));
    ui->client2_state->setText(QString::fromStdString(client_state[clientState[1]]));
    ui->client2_crawled_count->setText(QString::number(clientCrawl[1]));
    ui->client2_failure_count->setText(QString::number(clientFail[1]));
    if (clientState[1] == 1) {
        ui->client2_begin_time->setText(time2qstring(&clientBegin[1]));
        ui->client2_crawled_time->setText(QString::number(diff));
    }

    diff = difftime(mktime(now), mktime(&clientBegin[2]));
    ui->client3_name->setText(QString::fromStdString(clientC_name));
    ui->client3_state->setText(QString::fromStdString(client_state[clientState[2]]));
    ui->client3_crawled_count->setText(QString::number(clientCrawl[2]));
    ui->client3_failure_count->setText(QString::number(clientFail[2]));
    if (clientState[2] == 1) {
        ui->client3_begin_time->setText(time2qstring(&clientBegin[2]));
        ui->client3_crawled_time->setText(QString::number(diff));
    }
}

/** Utility function */
QString MainWindow::date2qstring(std::tm *tt) {
    std::string text = date[tt->tm_wday] + ", " + month[tt->tm_mon] + " " + std::to_string(tt->tm_mday) + ", " + std::to_string(tt->tm_year+1900);
    return QString::fromStdString(text);
}

QString MainWindow::time2qstring(std::tm *tt) {
    std::string text = std::to_string(tt->tm_hour + 1) + " : " + std::to_string(tt->tm_min + 1) + " : " + std::to_string(tt->tm_sec + 1);
    return QString::fromStdString(text);
}

/** Public get function */
QString MainWindow::getTime(){
    return ui->time->text();
}

