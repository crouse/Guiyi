#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // UserName, PassWord
    {
        lineEditUserName = new QLineEdit;
        lineEditPassWord = new QLineEdit;
        lineEditHostIp = new QLineEdit;

        lineEditUserName->setFixedSize(100, 20);
        lineEditPassWord->setFixedSize(100, 20);
        lineEditHostIp->setFixedSize(100, 20);
        lineEditHostIp->setText("127.0.0.1");

        lineEditUserName->setPlaceholderText(" 用户名：");
        lineEditPassWord->setPlaceholderText(" 密码：");
        lineEditHostIp->setPlaceholderText(" 服务器IP: ");
        lineEditPassWord->setEchoMode(QLineEdit::PasswordEchoOnEdit);

        ui->mainToolBar->addWidget(lineEditUserName);
        ui->mainToolBar->addSeparator();

        ui->mainToolBar->addWidget(lineEditPassWord);
        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addWidget(lineEditHostIp);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setTable(QString tableName, QSqlTableModel *&queryModel,
                          QTableView *&tableView, QSqlTableModel::EditStrategy editStrategy)
{
    QSqlQuery query;
    QString sql = QString("show full columns from `%1`").arg(tableName);
    query.exec(sql);

    queryModel = new QSqlTableModel(this);
    queryModel->setTable(tableName);
    queryModel->setEditStrategy(editStrategy);
    queryModel->setSort(0, Qt::AscendingOrder);

    int i = 0;
    while(query.next()) {
        QString comment = query.value(8).toString();
        queryModel->setHeaderData(i, Qt::Horizontal, comment);
        i++;
    }

    //queryModel->select();

    tableView->setModel(queryModel);
    tableView->setColumnHidden(0, true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->reset();
}

bool MainWindow::connDatabase(QString hostname, QString username, QString password, QString dbname)
{
    connDbHandle = QSqlDatabase::addDatabase("QMYSQL");
    connDbHandle.setDatabaseName(dbname);
    connDbHandle.setHostName(hostname);
    connDbHandle.setUserName(username);
    connDbHandle.setPassword(password);

    if(!connDbHandle.open()) return false;

    ui->actionCon->setDisabled(true);

    setTable("tb_guiyi", queryModel, ui->tableView, QSqlTableModel::OnFieldChange);

    return true;
}

void MainWindow::searchData()
{
    QString filter = " 1 = 1  ";
    QObjectList list = ui->page->children();
    qDebug() << list.length();
    QLineEdit *lineEditTmp;
    foreach(QObject *obj, list) {
        lineEditTmp = qobject_cast<QLineEdit*>(obj);
        if (lineEditTmp) {
            qDebug() << lineEditTmp->objectName() << lineEditTmp->text().trimmed();
            QString objName = lineEditTmp->objectName();
            QString objValue = lineEditTmp->text().trimmed();
            if (objValue.isEmpty()) continue;
            if (objName.startsWith("lineEditLike_")) {
                QString name = objName.mid(13);
                filter += QString(" and %1 like '%%2%' ").arg(name).arg(objValue);
            } else if (objName.startsWith("lineEdit_")) {
                QString name = objName.mid(9);
                filter += QString(" and %1 = '%2' ").arg(name).arg(objValue);
            }
        }
    }

    qDebug() << filter;

    queryModel->setFilter(filter);
    queryModel->select();
    ui->tableView->reset();
}

void MainWindow::on_actionCon_triggered()
{
    QString hostname = lineEditHostIp->text().trimmed();
    QString username = "root";
    QString password = "tbontBtiaQ@!";
    QString dbname = "guiyi";
    connDatabase(hostname, username, password, dbname);
    lineEditHostIp->setDisabled(true);
}

void MainWindow::on_actionQuery_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_pushButton_clicked()
{
    searchData();
}

QByteArray MainWindow::getImageFromDb(QString code)
{
    QSqlQuery query;
    QByteArray imageByteArray;
    query.prepare("select image from tb_images where code = :code");
    query.bindValue(":code", code);
    query.exec();
    while(query.next()) {
        imageByteArray = query.value(0).toByteArray();
    }

    return imageByteArray;
}

void MainWindow::showImage(QString code)
{
    QPixmap photo;
    QByteArray bytes = getImageFromDb(code);
    photo.loadFromData(bytes);
    QDialog *dialog = new QDialog();
    QGridLayout *layout = new QGridLayout();
    QLabel *label = new QLabel(this);
    label->setPixmap(photo);
    layout->addWidget(label);
    dialog->setLayout(layout);
    dialog->exec();

    delete label;
    delete layout;
    delete dialog;
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
#define GUIYI_CODE_LEN 11
    QString tmp = index.data().toString();
    if (tmp.length() == GUIYI_CODE_LEN && (tmp.startsWith("A") || tmp.startsWith("B"))) {
        // Show images
        qDebug() << tmp;
        showImage(tmp);
    }
}
