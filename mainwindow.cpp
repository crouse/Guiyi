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

    ui->stackedWidget->setCurrentIndex(0);
    ui->lineEdit_name->setFocus();
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

void MainWindow::clearSearchEdits()
{
   QObjectList list = ui->page->children();
   QLineEdit *tmp;
   foreach(QObject *obj, list) {
       tmp = qobject_cast<QLineEdit*>(obj);
       if (tmp) {
           tmp->clear();
       }
   }
   ui->lineEdit_name->setFocus();
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

    clearSearchEdits();
}

void MainWindow::on_actionCon_triggered()
{
    QString hostname = lineEditHostIp->text().trimmed();
    QString username = "guiyi";
    QString password = "guiyi";
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

void MainWindow::on_printImagetoPng_clicked()
{
    QSqlQuery query;
    query.prepare("select image from tb_images where code = :code");
    query.bindValue(":code", gCode);

    query.exec();
    while (query.next()) {
        QString filename = QFileDialog::getSaveFileName(this, tr("保存照片"), gCode, tr("*.png"));
        QPixmap photo;
        photo.loadFromData(query.value(0).toByteArray(), "PNG");
        photo.save(filename);
    }

    gCode = "";
}

void MainWindow::on_printInfotoPdf_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("保存个人档案"), gCode, tr("*.pdf"));
    QSqlQuery query;
    query.prepare("select name, gender, fahui_name, mod_time, birthday, race, province, city, district, degree, "
                  " graduate_time, graduate_school, hobby, job, year2start_learning_buddhism, "
                  " deep_understanding_of_dharma, maxim from tb_guiyi "
                  " where code = :code");

    query.bindValue(":code", gCode);
    query.exec();

    qDebug() << query.lastQuery() << query.lastError().text();

    QFile pdfFile(filename);
    pdfFile.open(QIODevice::WriteOnly);
    QPdfWriter *pdfWriter = new QPdfWriter(&pdfFile);
    QPainter *pdfPainter = new QPainter(pdfWriter);

    QFont font;
    font.setFamily("楷体");
    font.setPointSize(8);
    pdfPainter->setFont(font);

    if (query.next()) {
        QString fahuiName = query.value(2).toString();
        QString dateTime = query.value(3).toDateTime().toString("yyyy-MM-dd");
        pdfPainter->drawText(QRect(3000, 0, 6000, 300), QString("*** %1 %2 档案资料 【绝密，如有泄露因果自负】***").arg(fahuiName).arg(dateTime));

        QFont font;
        font.setFamily("宋体");
        font.setPointSize(12);
        pdfPainter->setFont(font);

        QPixmap photo;
        QByteArray bytes = getImageFromDb(gCode);
        photo.loadFromData(bytes);
        pdfPainter->drawPixmap(1200, 800, 885, 1239, photo);
        pdfPainter->setPen(QPen(Qt::gray, 10));
        pdfPainter->drawLine(QPoint(2200, 300), QPoint(2200, 15000));
        pdfPainter->drawLine(QPoint(2200, 300), QPoint(10000, 300));


        QString name = query.value(0).toString();
        QString gender = query.value(1).toString();
        QString birth = query.value(4).toString();
        QString race = query.value(5).toString();

        pdfPainter->setPen(QPen(Qt::black, 10));
        pdfPainter->drawText(QRect(2500, 900, 4000, 300), QString("[基本信息]"));
        pdfPainter->drawText(QRect(2500, 1200, 4000, 300), QString("姓名: %1").arg(name));
        pdfPainter->drawText(QRect(2500, 1500, 4000, 300), QString("姓别: %1").arg(gender));
        pdfPainter->drawText(QRect(2500, 1800, 4000, 300), QString("生日: %1").arg(birth));
        pdfPainter->drawText(QRect(2500, 2100, 4000, 300), QString("民族: %1").arg(race));

        pdfPainter->setPen(QPen(Qt::gray, 10));
        pdfPainter->drawLine(QPoint(2200, 3000), QPoint(10000, 3000));

        QString province = query.value(6).toString();
        QString city = query.value(7).toString();
        QString district = query.value(8).toString();

        pdfPainter->setPen(QPen(Qt::black, 10));
        pdfPainter->drawText(QRect(2500, 3900, 4000, 300), QString("[居住信息]"));
        pdfPainter->drawText(QRect(2500, 4200, 4000, 300), QString("省份: %1").arg(province));
        pdfPainter->drawText(QRect(2500, 4500, 4000, 300), QString("地市: %1").arg(city));
        pdfPainter->drawText(QRect(2500, 4800, 4000, 300), QString("区县: %1").arg(district));

        pdfPainter->setPen(QPen(Qt::gray, 10));
        pdfPainter->drawLine(QPoint(2200, 5700), QPoint(10000, 5700));

        QString degree = query.value(9).toString();
        QString graduate_time = query.value(10).toString();
        QString graduate_school = query.value(11).toString();
        QString hobby = query.value(12).toString();
        QString job = query.value(13).toString();

        pdfPainter->setPen(QPen(Qt::black, 10));
        pdfPainter->drawText(QRect(2500, 6600, 4000, 300), QString("[教育工作信息]"));
        pdfPainter->drawText(QRect(2500, 6900, 4000, 300), QString("学历: %1").arg(degree));
        pdfPainter->drawText(QRect(2500, 7200, 4000, 300), QString("毕业日期: %1").arg(graduate_time));
        pdfPainter->drawText(QRect(2500, 7500, 4000, 300), QString("毕业学校: %1").arg(graduate_school));
        pdfPainter->drawText(QRect(2500, 7800, 4000, 300), QString("爱好特长: %1").arg(hobby));
        pdfPainter->drawText(QRect(2500, 8100, 4000, 300), QString("工作: %1").arg(job));

        pdfPainter->setPen(QPen(Qt::gray, 10));
        pdfPainter->drawLine(QPoint(2200, 9000), QPoint(10000, 9000));

        QString startYear = query.value(14).toString();
        QString deepH = query.value(15).toString();
        QString maxim = query.value(16).toString();
        pdfPainter->setPen(QPen(Qt::black, 10));
        pdfPainter->drawText(QRect(2500, 9900, 4000, 300), QString("[学佛信息]"));
        pdfPainter->drawText(QRect(2500, 10200, 4000, 300), QString("学佛始年: %1").arg(startYear));
        pdfPainter->drawText(QRect(2500, 10500, 4000, 300), QString("学佛深度: %1").arg(deepH));
        pdfPainter->drawText(QRect(2500, 10800, 4000, 300), QString("个人格言: %1").arg(maxim));

    }

    pdfPainter->end();
    delete pdfWriter;
    delete pdfPainter;
    pdfFile.close();
    gCode = "";
}


void MainWindow::showImage(QString code)
{
    QPixmap photo;
    QByteArray bytes = getImageFromDb(code);
    photo.loadFromData(bytes);
    QDialog *dialog = new QDialog();
    QVBoxLayout *layout = new QVBoxLayout();
    QLabel *label = new QLabel(this);
    QPixmap p = photo.scaledToHeight(350, Qt::SmoothTransformation);
    label->setScaledContents(true);
    label->setPixmap(p);
    layout->addWidget(label);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *printImagetoPng = new QPushButton("导出照片");
    QPushButton *printInfotoPdf = new QPushButton("导出档案");

    buttonLayout->addWidget(printImagetoPng);
    buttonLayout->addWidget(printInfotoPdf);
    layout->addLayout(buttonLayout);

    gCode = code;
    connect(printImagetoPng, SIGNAL(clicked(bool)), this, SLOT(on_printImagetoPng_clicked()));
    connect(printInfotoPdf, SIGNAL(clicked(bool)), this, SLOT(on_printInfotoPdf_clicked()));

    dialog->setLayout(layout);
    dialog->exec();

    delete label;
    delete printImagetoPng;
    delete printInfotoPdf;
    delete buttonLayout;
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
