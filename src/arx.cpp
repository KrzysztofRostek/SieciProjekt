#include "arx.h"
#include <QMessageBox>
#include "ui_arx.h"
#include <vector>

ARX::ARX(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ARX)
{
    ui->setupUi(this);
    this->setWindowTitle("Model ARX");
    scrollAreaA = new QScrollArea(this);
    scrollAreaA->setWidgetResizable(true);
    scrollAreaA->setFrameShape(QFrame::NoFrame);

    scrollContentA = new QWidget();

    layoutInsideScrollA = new QVBoxLayout(scrollContentA);
    layoutInsideScrollA->setAlignment(Qt::AlignTop);

    scrollAreaA->setWidget(scrollContentA);

    ui->layout_A->addWidget(scrollAreaA);

    scrollAreaB = new QScrollArea(this);
    scrollAreaB->setWidgetResizable(true);
    scrollAreaB->setFrameShape(QFrame::NoFrame);

    scrollContentB = new QWidget();

    layoutInsideScrollB = new QVBoxLayout(scrollContentB);
    layoutInsideScrollB->setAlignment(Qt::AlignTop);

    scrollAreaB->setWidget(scrollContentB);

    ui->layout_B->addWidget(scrollAreaB);

    read_conf();
}

ARX::~ARX()
{
    delete ui;
}

void ARX::on_add_A_button_clicked()
{
    QDoubleSpinBox *spin_box_A = new QDoubleSpinBox(scrollContentA);

    spin_box_A->setRange(-1000.0, 1000.0);
    spin_box_A->setDecimals(2);
    spin_box_A->setSingleStep(0.1);

    layoutInsideScrollA->addWidget(spin_box_A);
    listA.append(spin_box_A);

    QScrollBar *sb = scrollAreaA->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void ARX::on_add_B_button_clicked()
{
    QDoubleSpinBox *spin_box_B = new QDoubleSpinBox(scrollContentB);

    spin_box_B->setRange(-1000.0, 1000.0);
    spin_box_B->setDecimals(2);
    spin_box_B->setSingleStep(0.1);

    layoutInsideScrollB->addWidget(spin_box_B);
    listB.append(spin_box_B);

    QScrollBar *sb1 = scrollAreaB->verticalScrollBar();
    sb1->setValue(sb1->maximum());
}

void ARX::on_buttonBox_accepted()
{
    if (listA.isEmpty()) {
        QMessageBox::warning(this, "BŁAD", "Lista współczynników A nie może być pusta.");
        return;
    }
    if (listB.isEmpty()) {
        QMessageBox::warning(this, "BŁAD", "Lista współczynników B nie może być pusta.");
        return;
    }

    std::vector<double> vecA;
    std::vector<double> vecB;
    for (auto *sb : listA) {
        vecA.push_back(sb->value());
    }

    for (auto *sb : listB) {
        vecB.push_back(sb->value());
    }

    int opoznienie = ui->opz_standard_doubleSpinBox->value();
    double szumy = ui->szumy_doubleSpinBox->value();

    emit s_update_model_params(vecA, vecB, opoznienie, szumy);

    this->accept();
    save_conf();
}

void ARX::on_buttonBox_rejected()
{
    this->reject();
}

void ARX::save_conf()
{
    QSettings settings("PK", "ARX");

    settings.setValue("ograniczenia_aktiv", ui->ogran_checkBox->isChecked());
    if (ui->ogran_checkBox->isChecked()) {
        settings.setValue("Max_u", ui->maxU_doubleSpinBox->value());
        settings.setValue("Min_u", ui->minU_doubleSpinBox->value());
        settings.setValue("Min_y", ui->minY_doubleSpinBox->value());
        settings.setValue("Max_y", ui->maxY_doubleSpinBox->value());
    }

    settings.setValue("opoznienie", ui->opz_standard_doubleSpinBox->value());
    settings.setValue("szumy", ui->szumy_doubleSpinBox->value());

    //A

    settings.beginWriteArray("WspolczynnikiA");
    for (int i = 0; i < listA.size(); ++i) {
        settings.setArrayIndex(i);

        settings.setValue("val", listA[i]->value());
    }
    settings.endArray();

    //B
    settings.beginWriteArray("WspolczynnikiB");
    for (int i = 0; i < listB.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("val", listB[i]->value());
    }
    settings.endArray();
}

void ARX::read_conf()
{
    QSettings settings("PK", "ARX");

    bool first = !settings.contains("opoznienie");

    ui->ogran_checkBox->setChecked(settings.value("ograniczenia_aktiv", true).toBool());
    ui->maxU_doubleSpinBox->setValue(settings.value("Max_u", 10.0).toDouble());
    ui->minU_doubleSpinBox->setValue(settings.value("Min_u", -10.0).toDouble());
    ui->maxY_doubleSpinBox->setValue(settings.value("Max_y", 10.0).toDouble());
    ui->minY_doubleSpinBox->setValue(settings.value("Min_y", -10.0).toDouble());

    ui->opz_standard_doubleSpinBox->setValue(settings.value("opoznienie", 1.0).toDouble());
    ui->szumy_doubleSpinBox->setValue(settings.value("szumy", 0.0).toDouble());

    //A
    qDeleteAll(listA);
    listA.clear();
    if (first) {
        for (int i = 0; i < 3; ++i) {
            on_add_A_button_clicked();
        }
    }

    int sizeA = settings.beginReadArray("WspolczynnikiA");
    for (int i = 0; i < sizeA; ++i) {
        settings.setArrayIndex(i);
        double val = settings.value("val").toDouble();

        on_add_A_button_clicked();

        if (!listA.isEmpty()) {
            listA.last()->setValue(val);
        }
    }
    settings.endArray();

    //B

    qDeleteAll(listB);
    listB.clear();

    if (first) {
        for (int i = 0; i < 3; ++i) {
            on_add_B_button_clicked();
        }
    }

    int sizeB = settings.beginReadArray("WspolczynnikiB");
    for (int i = 0; i < sizeB; ++i) {
        settings.setArrayIndex(i);
        double val = settings.value("val").toDouble();

        on_add_B_button_clicked();

        if (!listB.isEmpty()) {
            listB.last()->setValue(val);
        }
    }
    settings.endArray();
}

void ARX::set_parametry(const std::vector<double> &A,
                        const std::vector<double> &B,
                        int k,
                        double szumy)
{
    qDeleteAll(listA);
    listA.clear();

    for (double val : A) {
        on_add_A_button_clicked();
        if (!listA.isEmpty())
            listA.last()->setValue(val);
    }

    qDeleteAll(listB);
    listB.clear();

    for (double val : B) {
        on_add_B_button_clicked();
        if (!listB.isEmpty())
            listB.last()->setValue(val);
    }

    ui->opz_standard_doubleSpinBox->setValue((double) k);
    ui->szumy_doubleSpinBox->setValue(szumy);
}

void ARX::on_delA_pushButton_clicked()
{
    if (listA.size() <= 1) {
        QMessageBox::information(this, "Info", "Wymagany min. 1 współczynnik.");
        return;
    }

    QDoubleSpinBox *spin_box_to_remove = listA.takeLast();

    layoutInsideScrollA->removeWidget(spin_box_to_remove);

    delete spin_box_to_remove;
    spin_box_to_remove = nullptr;
}

void ARX::on_delB_pushButton_clicked()
{
    if (listB.size() <= 1) {
        QMessageBox::information(this, "Info", "Wymagany min. 1 współczynnik.");
        return;
    }

    QDoubleSpinBox *spin_box_to_remove = listB.takeLast();

    layoutInsideScrollB->removeWidget(spin_box_to_remove);

    delete spin_box_to_remove;
    spin_box_to_remove = nullptr;
}
