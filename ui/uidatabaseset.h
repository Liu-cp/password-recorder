#ifndef UIDATABASESET_H
#define UIDATABASESET_H

#include <QWidget>

namespace Ui {
class UiDatabaseSet;
}

class UiDatabaseSet : public QWidget
{
    Q_OBJECT

public:
    explicit UiDatabaseSet(QWidget *parent = nullptr);
    ~UiDatabaseSet();

private slots:
    void on_checkBox_showPwd_stateChanged(int arg1);
    void on_button_return_clicked();
    void on_button_dbTest_clicked();
    void on_button_save_clicked();

    void handleDatabaseSetSignal();

private:
    Ui::UiDatabaseSet *ui;
};

#endif // UIDATABASESET_H
