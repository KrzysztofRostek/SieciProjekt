#ifndef ARX_H
#define ARX_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>


#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QScrollBar>

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
namespace Ui {
class ARX;
}

class ARX : public QDialog
{
    Q_OBJECT

public:
    explicit ARX(QWidget *parent = nullptr);
    ~ARX();

signals:
   void s_update_model_params(std::vector<double> A, std::vector<double> B, int k, double s);

public:
    void set_parametry(const std::vector<double>& A, const std::vector<double>& B, int k, double szumy);

private slots:

    void on_add_A_button_clicked();
    void on_add_B_button_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void save_conf();

    void read_conf();

    void on_delA_pushButton_clicked();

    void on_delB_pushButton_clicked();

private:
    Ui::ARX *ui;
    QScrollArea *scrollAreaA;
    QWidget *scrollContentA;
    QVBoxLayout *layoutInsideScrollA;

    QScrollArea *scrollAreaB;
    QWidget *scrollContentB;
    QVBoxLayout *layoutInsideScrollB;

    QList<QDoubleSpinBox*> listA;
    QList<QDoubleSpinBox*> listB;
};

#endif // ARX_H
