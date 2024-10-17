#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QComboBox>
#include <QListWidget>
#include <QDialog>

class CustomComboBox final : public QComboBox
{
    Q_OBJECT
public:
    explicit CustomComboBox(QWidget *parent = nullptr);

    void addItems(const QStringList &texts);
    void clearItems();
    void setText(const QString &text);
    QString text() const;

protected:
    void showPopup() override;
    void hidePopup() override;

private:
    QListWidget *listWidget;
    QDialog *listDialog;
};

#endif // CUSTOMCOMBOBOX_H
