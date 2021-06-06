#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "highlighter.h"
#include "netmod.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QDebug>
#include <QStack>
#include <String>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDirIterator>
#include <QStandardItemModel>
#include <QTableView>
#include <ctime>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    jsonModel = new JsonTreeModel(this);
    ui->treeView->setModel(jsonModel);

    mod1 = new QStandardItemModel();
    root = "data";
    init();
    connect(ui->actionnew,&QAction::triggered,[=](){
        MainWindow *newMainWin = new MainWindow;
        newMainWin->show();
    });
    connect(ui->actionopen,&QAction::triggered,[=](){
        QString path = QFileDialog::getOpenFileName(this,"打开文件",root,"JSON(*.json)");
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        QByteArray array = file.readAll();
        ui->textEdit->setText(codec->toUnicode(array));
        file.close();
    });

    connect(ui->actionabout,&QAction::triggered,[=](){
        QMessageBox::about(this, tr("Jsondog"),
                           tr("<p style=\"color:green;font-size:18px;\"><b>JsonDog</b> 是一个可以用来编写json的云笔记工具 </p>\
                              <p style=\"color:green;font-size:18px;\">实现了json的可视化编写 并且可以储存的云端 </p>\
                <p style=\"color:green;font-size:18px;\">代码实现：SDU fastle </p>\
                <p style=\"color:green;font-size:18px;\">指导教师：戴圣 </p>"));

    });


    connect(ui->action1,&QAction::triggered,[=](){

        QMessageBox::about(this, tr("上传成功"),
                           tr("<p style=\"color:green;font-size:18px;\"><b>欧内的手 好汉！</b> </p>"));

    });
    connect(ui->action2,&QAction::triggered,[=](){

        QMessageBox::about(this, tr("获取成功"),
                           tr("<p style=\"color:green;font-size:18px;\"><b>大吉!!!!!!!!!!!</b></p>"));

    });

    connect(ui->actionsave,&QAction::triggered,[=](){
        QFileDialog fileDialog;
        QString fileName = fileDialog.getSaveFileName(this,tr("保存"),"./data",tr("Text File(*.json)"));
        if(fileName == "")
        {
            return;
        }
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this,tr("错误"),tr("打开文件失败"));
            return;
        }
        else   {
            QTextStream textStream(&file);
            QString str = ui->textEdit->toPlainText();

            textStream<<str;
            QMessageBox::warning(this,tr("提示"),tr("保存文件成功"));
            file.close();
            refresh();
        }
    });

    connect(ui->actionrefresh,&QAction::triggered,[=](){
        refresh();
    });

    connect(ui->actioncheck,&QAction::triggered,[=](){
        QString alltext = ui->textEdit->toPlainText();
        //  qDebug() << alltext << "\n";
        QString ans = check(alltext);
        /*  int len = ans.length();
        for(int i = 0; i < len; i++)
        {
            qDebug() << ans.at(i) << "\n";
            if(ans.at(i) == '\xa') qDebug() << "!" << "\n";
        }
*/
        if(ans.at(0) == 'C')
        {
            ans = "<font color=\"#7FB347\">" + ans + "</font>";
        }
        else ans = "<font color=\"#FF0000\">" + ans + "</font>";
        ui->textBrowser->setText(ans);
    });

}

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::init(){
    initLoadDump();
    setupEditer();
    initEdit();
    initBuildPath();
    initBuildTable();
    nt = new Netmod();
    nt->getConnect();
}

void MainWindow::refresh(){
    QString path = "./data";
    QDirIterator iter(path, QStringList() << "*.json",
                      QDir::Files | QDir::NoSymLinks,
                      QDirIterator::Subdirectories);

    int i = 0;
    while(iter.hasNext())
    {
        iter.next();
        mod1->setItem(i, 0, new QStandardItem(iter.fileName()));
        qDebug() << "filePath:" << iter.filePath();  //包含文件名的文件的全路径
        QPushButton *button = new QPushButton("打开");

        // set custom property
        button->setProperty("文件名", iter.filePath());

        // set click event
        connect(button, SIGNAL(clicked()), this, SLOT(onTableBtnClicked()));

        // notice every time insert the button at the last line
        ui->filetableView->setIndexWidget(mod1->index(mod1->rowCount() - 1, 1), button);
        i++;
    }

    ui->filetableView->setColumnWidth(0,200);
    ui->filetableView->setColumnWidth(1,90);
}


void MainWindow::setupEditer()
{
    editor = ui->textEdit;
    highlighter = new Highlighter(editor->document());

    QFile file("mainwindow.h");

    if (file.open(QFile::ReadOnly | QFile::Text))
        editor->setPlainText(file.readAll());
}


void MainWindow::initEdit()
{
    //增删部分参照示例：editable tree
    //
    //我的item默认为enum:none，但目前还没有delegate来自定义操作
    //只在item的appendchilren里测试性的改成了enum:value

    //添加节点
    connect(ui->btnInsert,&QPushButton::clicked,this,[this](){
        QModelIndex index = ui->treeView->selectionModel()->currentIndex();
        if(!index.isValid())
            return;
        QAbstractItemModel *model = ui->treeView->model();
        if(!model->insertRow(index.row()+1,index.parent()))
            return;
        updateIndex();
        //修改insert的内容
        /*for (int column = 0; column < model->columnCount(index.parent()); ++column) {
            QModelIndex child = model->index(index.row()+1, column, index.parent());
            model->setData(child, QVariant("[No data]"), Qt::EditRole);
        }*/
    });
    //添加子节点
    connect(ui->btnInsertChild,&QPushButton::clicked,this,[this](){
        QModelIndex index = ui->treeView->selectionModel()->currentIndex();
        QAbstractItemModel *model = ui->treeView->model();

        if (model->columnCount(index) == 0) {
            if (!model->insertColumn(0, index))
                return;
        }

        if (!model->insertRow(0, index))
            return;
        //修改insert的内容
        /*for (int column = 0; column < model->columnCount(index); ++column) {
            QModelIndex child = model->index(0, column, index);
            model->setData(child, QVariant("[No data]"), Qt::EditRole);
        }*/
        ui->treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                                        QItemSelectionModel::ClearAndSelect);
        updateIndex();
    });
    //删除节点树
    connect(ui->btnRemove,&QPushButton::clicked,this,[this](){
        QModelIndex index = ui->treeView->selectionModel()->currentIndex();
        QAbstractItemModel *model = ui->treeView->model();
        if (model->removeRow(index.row(), index.parent()))
            updateIndex();
    });
}

void MainWindow::initLoadDump()
{
    //生成树


    connect(ui->actionbuildtree,&QAction::triggered,this,[this](){
        const QString alltext1 = ui->textEdit->toPlainText();

        jsonModel->loadJson(alltext1);
        ui->treeView->expandAll();
    });

    connect(ui->actionbuildcode,&QAction::triggered,this,[this](){
        QString now = jsonModel->dumpJson();
        ui->textEdit->setText(now);
    });

    /*
    connect(ui->btnLoadPath,&QPushButton::clicked,this,[this](){
        const QString jsonpath = QFileDialog::getOpenFileName(this,"File Path");
        if(jsonpath.isEmpty()) return;
        ui->editLoadPath->setText(jsonpath);
    });
    //导入Json文件
    connect(ui->btnLoadJson,&QPushButton::clicked,this,[this](){
        const QString loadpath = ui->editLoadPath->text();
        if(loadpath.isEmpty()) return;
        //parseJson.loadJson(loadpath);
        jsonModel->loadJson(loadpath);
        ui->treeView->expandAll();
    });

    //选择导出的文件路径
    connect(ui->btnDumpPath,&QPushButton::clicked,this,[this](){
        const QString jsonpath = QFileDialog::getSaveFileName(this,"Save Path");
        if(jsonpath.isEmpty()) return;
        ui->editDumpPath->setText(jsonpath);
    });
    //导出Json文件
    connect(ui->btnDumpJson,&QPushButton::clicked,this,[this](){
        const QString dumppath = ui->editDumpPath->text();
        if(dumppath.isEmpty()) return;
        //parseJson.dumpJson(dumppath);
        jsonModel->dumpJson(dumppath);
    });*/
}


void MainWindow::onTableBtnClicked()
{
    QPushButton *button = (QPushButton *)sender();
    qDebug() << button->property("文件名").toString();
    QFile file(button->property("文件名").toString());
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    file.open(QIODevice::ReadOnly);
    QByteArray array = file.readAll();
    ui->textEdit->setText(codec->toUnicode(array));
    file.close();

}

void MainWindow::initBuildPath()
{
    QDir *temp = new QDir("./");
    bool exist = temp->exists(root);
    if(!exist)
    {
        temp->mkdir(root);
    }
    //  else {
    //    qDebug() << " fuck!";
    // }
}

void MainWindow::initBuildTable(){
    ui->filetableView->setModel(this->mod1);
    mod1->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("文件名")));

    mod1->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("添加")));
    ui->filetableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->filetableView->verticalHeader()->hide();
    ui->filetableView->setColumnWidth(0,200);
    ui->filetableView->setColumnWidth(1,90);


    refresh();
}

void MainWindow::updateIndex()
{
    //抄的示例
    //bool hasSelection = !ui->treeView->selectionModel()->selection().isEmpty();
    //removeRowAction->setEnabled(hasSelection);
    //removeColumnAction->setEnabled(hasSelection);

    bool hasCurrent = ui->treeView->selectionModel()->currentIndex().isValid();
    //insertRowAction->setEnabled(hasCurrent);
    //insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent) {
        ui->treeView->closePersistentEditor(ui->treeView->selectionModel()->currentIndex());

        int row = ui->treeView->selectionModel()->currentIndex().row();
        int column = ui->treeView->selectionModel()->currentIndex().column();
        if (ui->treeView->selectionModel()->currentIndex().parent().isValid())
            qDebug()<<tr("Position: (%1,%2)").arg(row).arg(column);
        else
            qDebug()<<tr("Position: (%1,%2) in top level").arg(row).arg(column);
    }
}

QString MainWindow::check(QString x){
    QStack<QChar> st;
    int len = x.length();
    int line = 0, p = 0;
    bool flag = false;
    QString type = "{\"}[]\\";
    qDebug() << "!" << "\n";
    QString str = x;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    QByteArray ba = doc.toJson(QJsonDocument::Indented);
    if(!ba.isEmpty()) {
        return "Correct format";
    }
    while(p < len)
    {

        qDebug() << x.at(p) << " :";
        qDebug() << type.indexOf(x.at(p)) << "\n";
        switch (type.indexOf(x.at(p))) {
        case 0:
            if(!st.empty() && st.top() == '\"') break;
            p++;
            flag = true;
            while(p < len && (x.at(p) == ' ' || x.at(p) == '\xa' || x.at(p) == '\t'))
            {
                qDebug() << x.at(p) << "\n";
                if(x.at(p) == '\xa' || x.at(p) == '\n')
                {
                    line++;
                }
                p++;

            }
            if(x.at(p) == '"')
            {
                p--;
                st.push('{');
            }
            else
            {
                return "[Error]: on line" + QString(line) + "::cannot find \" at" + x.at(p) ;
            }
            break;
        case 1:

            flag = true;
            if(st.empty() || st.top() != '"')
            {
                st.push(x.at(p));
            }
            else
            {
                st.pop();
            }
            break;
        case 3:
            if(!st.empty() && st.top() == '\"') break;
            flag = true;
            st.push(x.at(p));
            break;
        case 4:
            if(!st.empty() && st.top() == '\"') break;
            flag = true;
            if(st.empty() || st.top() != '[')
            {
                return "[Error]: on line" + QString(line) + "::cannot find ] at" + x.at(p);
            }
            else
            {
                st.pop();
            }
            break;
        case 2:
            if(!st.empty() && st.top() == '\"') break;
            flag = true;
            if(st.empty() || st.top() != '{')
            {
                return "[Error]: on line " + QString(line) + "::cannot find } at" + x.at(p);
            }
            else
            {
                st.pop();
            }
            break;
        case 5:
            flag = true;
            p++;
            break;
        default:
            if(x.at(p) == '\xa' || x.at(p) == '\n') line++;
        }
        p++;
    }
    if(!flag) return "[Error] : empty!";
    if(ba.isEmpty())
    {
        return "[Error] : Missing delimiter like \',\' or other problem";
    }

    return "Correct format";
}
