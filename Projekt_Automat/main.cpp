#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <vector>
#include <QTableWidget>

class Produkt {
private:
    std::string nazwa;
    double cena;
    int ilosc;

public:
    Produkt(const std::string& nazwa, double cena, int ilosc)
        : nazwa(nazwa), cena(cena), ilosc(ilosc) {}

    std::string getNazwa() const { return nazwa; }
    double getCena() const { return cena; }
    int getIlosc() const { return ilosc; }

    void setNazwa(const std::string& n) { nazwa = n; }
    void setCena(double c) { cena = c; }
    void setIlosc(int i) { ilosc = i; }

    void zmniejszIlosc()
    {
        if (ilosc > 0)
            --ilosc;
        else
            throw std::out_of_range("Brak produktu");
    }
};

class PortfelKarta {
public:
    PortfelKarta(const QString& numerKarty, const QString& nazwaPosiadacza, double kwotaNaKoncie)
        : numerKarty_(numerKarty), nazwaPosiadacza_(nazwaPosiadacza), kwotaNaKoncie_(kwotaNaKoncie) {}

    const QString& getNumerKarty() const {
        return numerKarty_;
    }

    const QString& getNazwaPosiadacza() const {
        return nazwaPosiadacza_;
    }

    double getKwotaNaKoncie() const {
        return kwotaNaKoncie_;
    }

    void setKwotaNaKoncie(double kwota) {
        kwotaNaKoncie_ = kwota;
    }

private:
    QString numerKarty_;
    QString nazwaPosiadacza_;
    double kwotaNaKoncie_;
};


std::vector<Produkt> wczytajNapoje(const std::string& nazwaPliku)
{
    std::vector<Produkt> napoje;

    QFile file(nazwaPliku.c_str());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, "Błąd", "Nie można otworzyć pliku.");
        return napoje;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList tokens = line.split(' ');
        if (tokens.size() == 3)
        {
            std::string nazwa = tokens[0].toStdString();
            double cena = tokens[1].toDouble();
            int ilosc = tokens[2].toInt();
            Produkt napoj(nazwa, cena, ilosc);
            napoje.push_back(napoj);
        }
    }

    file.close();

    return napoje;
}

void dodajNapoj(Produkt& napoj, const std::string& nazwaPliku)
{
    QFile file(nazwaPliku.c_str());
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, "Błąd", "Nie można otworzyć pliku.");
        return;
    }

    QTextStream out(&file);
    out << napoj.getNazwa().c_str() << " " << napoj.getCena() << " " << napoj.getIlosc() << "\n";

    file.close();
}

void usunNapoj(int pozycja, const std::string& nazwaPliku)
{
    QFile file(nazwaPliku.c_str());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, "Błąd", "Nie można otworzyć pliku.");
        return;
    }

    QString tempFileName = "temp.txt"; // Tymczasowy plik do zapisu
    QFile tempFile(tempFileName);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, "Błąd", "Nie można zapisać pliku.");
        file.close();
        return;
    }

    QTextStream in(&file);
    QTextStream out(&tempFile);

    int lineCount = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (lineCount != pozycja)
            out << line << "\n";
        ++lineCount;
    }

    file.close();
    tempFile.close();

    if (QFile::remove(nazwaPliku.c_str()))
    {
        if (!QFile::rename(tempFileName, nazwaPliku.c_str()))
            QMessageBox::critical(nullptr, "Błąd", "Nie można zmienić nazwy pliku.");
    }
    else
    {
        QMessageBox::critical(nullptr, "Błąd", "Nie można usunąć pliku.");
        tempFile.remove();
    }
}


void serwisant(const std::string& nazwaPliku)
{
    PortfelKarta karta1("1234567890123456", "Jan Kowalski", 500.0);

    QDialog dialog;
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Lista napojów
    QListWidget* listaNapojow = new QListWidget(&dialog);
    layout->addWidget(listaNapojow);

    std::vector<Produkt> napoje = wczytajNapoje(nazwaPliku);
    for (const auto& napoj : napoje)
    {
        QString itemText = "Nazwa napoju: " + QString::fromStdString(napoj.getNazwa()) + ", cena: " + QString::number(napoj.getCena()) + " zł, ilość: " + QString::number(napoj.getIlosc()) + " sztuk";
        listaNapojow->addItem(itemText);
    }

    QLabel* nazwaLabel = new QLabel("Nazwa:", &dialog);
    QLineEdit* nazwaEdit = new QLineEdit(&dialog);
    layout->addWidget(nazwaLabel);
    layout->addWidget(nazwaEdit);

    QLabel* cenaLabel = new QLabel("Cena:", &dialog);
    QLineEdit* cenaEdit = new QLineEdit(&dialog);
    layout->addWidget(cenaLabel);
    layout->addWidget(cenaEdit);

    QLabel* iloscLabel = new QLabel("Ilość:", &dialog);
    QLineEdit* iloscEdit = new QLineEdit(&dialog);
    layout->addWidget(iloscLabel);
    layout->addWidget(iloscEdit);

    QPushButton* dodajButton = new QPushButton("Dodaj napój", &dialog);
    QPushButton* usunButton = new QPushButton("Usuń napój", &dialog);
    layout->addWidget(dodajButton);
    layout->addWidget(usunButton);

    QObject::connect(dodajButton, &QPushButton::clicked, [&]() {
        std::string nazwa = nazwaEdit->text().toStdString();
        double cena = cenaEdit->text().toDouble();
        int ilosc = iloscEdit->text().toInt();

        Produkt napoj(nazwa, cena, ilosc);
        dodajNapoj(napoj, nazwaPliku);

        QString itemText = "Nazwa napoju: " + QString::fromStdString(nazwa) + ", cena: " + QString::number(cena) + " zł, ilość: " + QString::number(ilosc) + " sztuk";
        listaNapojow->addItem(itemText);
    });

    QObject::connect(usunButton, &QPushButton::clicked, [&]() {
        int selectedRow = listaNapojow->currentRow();
        if (selectedRow >= 0) {
            listaNapojow->takeItem(selectedRow);
            usunNapoj(selectedRow, nazwaPliku);
        }
    });

    dialog.exec();
}





int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QMainWindow mainWindow;
    QWidget* mainWidget = new QWidget(&mainWindow);
    QGridLayout* mainLayout = new QGridLayout(mainWidget);
    mainWindow.resize(800, 600);

    std::vector<Produkt> napoje = wczytajNapoje("lista_napojow.txt");

    int numRows = 5; // Liczba wierszy w siatce
    int numColumns = 4; // Liczba kolumn w siatce

    int currentRow = 0;
    int currentColumn = 0;

    QLabel* napojeLabel = new QLabel("Odbiór napoju:", mainWidget);
    QFont font = napojeLabel->font();
    font.setBold(true);
    font.setPointSize(20);
    napojeLabel->setFont(font);
    mainLayout->addWidget(napojeLabel, numRows + 2, 0, 1, numColumns, Qt::AlignCenter);

    QPushButton* platnoscKartaButton = new QPushButton("Platnosc karta");
    mainLayout->addWidget(platnoscKartaButton, 2, numColumns + 1);

    QObject::connect(platnoscKartaButton, &QPushButton::clicked, [&]() {
        // Obsługa płatności kartą
        QMessageBox::information(nullptr, "Platnosc karta", "Wybrano platnosc karta");
    });

    QPushButton* platnoscGotowkaButton = new QPushButton("Platnosc gotowka");
    mainLayout->addWidget(platnoscGotowkaButton, 2, numColumns + 2);

    QObject::connect(platnoscGotowkaButton, &QPushButton::clicked, [&]() {
        // Obsługa płatności gotówką
        QMessageBox::information(nullptr, "Platnosc gotowka", "Wybrano platnosc gotowka");
    });

    QPushButton* serwisButton = new QPushButton("Serwis");
    mainLayout->addWidget(serwisButton, numRows + 2, numColumns + 1, 1, 2);

    QObject::connect(serwisButton, &QPushButton::clicked, [&]() {
        serwisant("lista_napojow.txt");
    });

    QTextEdit* wyswietlaczTekstowy = new QTextEdit(mainWidget);
    wyswietlaczTekstowy->setReadOnly(true); // Ustawienie trybu tylko do odczytu
    mainLayout->addWidget(wyswietlaczTekstowy, 0, numColumns + 1, 2, 2);

    for (int i = 0; i < napoje.size(); i++)
    {
        const Produkt& napoj = napoje[i];
        QString buttonText = QString::fromStdString(napoj.getNazwa()) + "\nCena: " + QString::number(napoje[i].getCena()) + " zl\nIlosc: " + QString::number(napoje[i].getIlosc());
        QPushButton* button = new QPushButton(buttonText, mainWidget);
        mainLayout->addWidget(button, currentRow, currentColumn);

        QObject::connect(button, &QPushButton::clicked, [i, &napoje, wyswietlaczTekstowy, napojeLabel]() {
            try {
                napoje[i].zmniejszIlosc();
                QString infoText = "Odbierz zakupiony napój: " + QString::fromStdString(napoje[i].getNazwa());
                napojeLabel->setText(infoText);
                wyswietlaczTekstowy->setText("Cena: " + QString::number(napoje[i].getCena()) + " zl");
                QMessageBox::information(nullptr, "Sukces", "Napoj zakupiony!");
            } catch (const std::out_of_range& e) {
                QMessageBox::critical(nullptr, "Blad", e.what());
            }
        });

        currentColumn++;
        if (currentColumn == numColumns)
        {
            currentColumn = 0;
            currentRow++;
            if (currentRow == numRows)
                break; // Przerwij petle, jesli osiagnieto maksymalny rozmiar siatki
        }
    }

    mainWidget->setLayout(mainLayout);
    mainWindow.setCentralWidget(mainWidget);
    mainWindow.show();

    return a.exec();
}











