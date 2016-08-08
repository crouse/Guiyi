#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QLineEdit>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSql>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>
#include <QTableView>
#include <QFileDialog>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QByteArray>
#include <QComboBox>
#include <QCryptographicHash>
#include <QLabel>
#include <QGridLayout>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QLineEdit *lineEditUserName;
    QLineEdit *lineEditPassWord;
    QLineEdit *lineEditHostIp;

    QSqlDatabase connDbHandle;
    QSqlTableModel *queryModel;
    bool connDatabase(QString hostname, QString username, QString password, QString dbname);
    void setTable(QString tableName, QSqlTableModel *&queryModel, QTableView *&tableView, QSqlTableModel::EditStrategy editStrategy);

    QByteArray getImageFromDb(QString code);
    void searchData();
    void showImage(QString code);
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionCon_triggered();

    void on_actionQuery_triggered();

    void on_pushButton_clicked();

    void on_tableView_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
