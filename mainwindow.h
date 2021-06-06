#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "highlighter.h"
#include "jsontreemodel.h"
#include <QVector>
#include <QString>
#include <netmod.h>
#include <QTextEdit>
#include <QStandardItemModel>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setfonttype();
    QString check(QString x);

private:
    void init();
    void refresh();
    void setupEditer();
    void initEdit();
    void updateIndex();
    void initLoadDump();
    void initBuildPath();
    void initBuildTable();
    QTextEdit *editor;
    QStandardItemModel *mod1;
    Highlighter *highlighter;
    JsonTreeModel *jsonModel;
    Netmod *nt;
    Ui::MainWindow *ui;
    QString root;
private slots:
    void onTableBtnClicked();
};

#endif // MAINWINDOW_H
