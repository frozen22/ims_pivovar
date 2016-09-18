/**
 * Popis: Model pivovaru
 * Soubor: main.cpp
 * Autori: Frantisek Nemec (xnemec61)
 *         Jan Opalka (xopalk01)
 *
 * Vytvoreno: 2013-12-08
 */

#include <simlib.h>
#include <list>
#include <vector>
#include <time.h> // Seed for random
#include <iostream>
#include <string>

#include "AverageValue.h"
#include "MaterialStore.h"

//#define DEBUG

using std::list;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;

std::string HELP = "Model pivovaru\n"
    "Pouziti:\n"
    "   Bez zadani parametru, program pouzije vychozi hodnoty.\n"
    "   Vychozi hodnoty:\n"
    "      delka simulace jeden rok\n"
    "      exponencialni rozlozeni prichodu objednavek se streden 10 dni\n"
    "      velikost objednavky 5-25 varek ~ 12500-62500 litru piva\n"
    "\n"
    "   ./pivovar\n"
    "   ./pivovar 8760 e 336 5 15\n"
    "\n"
    "   ./pivovar delka_simulace rozlozeni_objednavek cas_objednavek min_varek max_varek\n"
    "\n"
    "Popis parametru:\n"
    "   Pokud chcete nektery parametr vynechat zadejte misto nej znak 'd'.\n"
    "   Vsechny ciselne hodnoty musi byt vetsi jako nula."
    "\n"
    "   delka_simulace - delka simulace v hodinach \n"
    "   rozlozeni prichodu objednavek:\n"
    "      'e' - exponencialni\n"
    "      'u' - rovnomerne\n"
    "   cas_objednavek - cas prichodu objednavek\n"
    "   min_varek - minimalni pocet varek v objednavce\n"
    "   max_varek - maximalni pocet varek v objednavce\n\n"; 


// Zakladni casovou jednotkou je hodina
#define MINUTA /60.0
#define DEN *24
#define TYDEN *168
#define MESIC *720
#define ROK *8760

/* Parametry programu */
double TIME_DELKA_SIMULACE = 1 ROK; 

// 'u' - roznomerne rozlozeni
// 'e' - exponencialni rozlozeni
char ROZLOZENI_PRICHODU_OBJEDNAVEK = 'e'; 

double TIME_PRICHOD_OBJEDNAVEK = 10 DEN;

unsigned int MIN_POCET_VAREK_V_OBJEDNAVCE = 5;
unsigned int MAX_POCET_VAREK_V_OBJEDNAVCE = 25;

/* Statistiky */
Histogram HistDokonceniVarek("Cas dokonceni varky",600,192,15);
Histogram HistDokonceniObjednavek("Cas potrebny na dokonceni objevnavky",600,384,10);
AverageValue VyuzitiSpilek;
int pocetDokoncenychLitru = 0;

/* Zakladni priority objednavek */
const int NORMALNI_PRIORITA = 1;
const int VYSOKA_PRIORITA = 2;

// Ulozeni poctu hotovych litru piva. Odtud objednavky odebiraji hotova piva.
long hotoveLitry[HIGHEST_PRIORITY];

/* Zarizeni */
Facility CekaniNaDokonceniObjednavky[HIGHEST_PRIORITY];
Facility PripravaVarky("Priprava varky");

/* Sklady */
Store Humna("Humna", 5); // Kliceni
Store Hvozdy("Hvozdy", 4); // Hvozdeni
Store Srotovnik("Srovnik", 1);
Store VystiraciKad("Vystiraci kad", 1);
Store VarnyKotel("Varny kotel", 1);
Store Spilka("Kade ve spilce", 6);
Store DokvasujiciTanky("Dokvasujici tanky", 25);
Store Filtry("Filtry", 3);
Store Stacirna("Stacirna", 1);

const unsigned int KAPACITA_SPILKY_VE_VARKACH = 3;
class PripravaVareni;
list<PripravaVareni*> SpilkyFronta;
Store PreSpilkaSklad("PreSpilka sklad", KAPACITA_SPILKY_VE_VARKACH);

/* Objednavky */
const unsigned int VELIKOST_VARKY = 2500; // litry

/* Suroviny */
const int JECMEN_KAPACITA = 4000; // kilogramy
const int JECMEN_GENEROVANI = 2000;
const int VODA_KAPACITA = 10000; // litry
const int VODA_GENEROVANI = 1260;
const int KVASINKY_KAPACITA = 5;
const int KVASINKY_GENEROVANI = 5;
const int CHMEL_KAPACITA = 1;
const int CHMEL_GENEROVANI = 1;

const double CAS_PRIPRAVY_JECMENE = 1;
const double CAS_PRIPRAVY_VODY = 1;
const double CAS_PRIPRAVY_KVASINEK = 1 DEN;
const double CAS_PRIPRAVY_CHMELE = 5 MINUTA;

MaterialStore SkladVody(CAS_PRIPRAVY_VODY, VODA_GENEROVANI, VODA_KAPACITA);
MaterialStore SkladJecmene(CAS_PRIPRAVY_JECMENE,JECMEN_GENEROVANI, JECMEN_KAPACITA);
MaterialStore SkladKvasinek(CAS_PRIPRAVY_KVASINEK, KVASINKY_GENEROVANI, KVASINKY_KAPACITA);
MaterialStore SkladChmele(CAS_PRIPRAVY_CHMELE, CHMEL_GENEROVANI, CHMEL_KAPACITA);

/* Spotreby surovin na varku */
const int SPOTREBA_JECMENE_KLICENI = 500;
const int SPOTREBA_VODY_KLICENI = 4000;
const int SPOTREBA_VODY_VYSTIRANI = 625;
const int SPOTREBA_VODY_CHMELENI = 875;
const int SPOTREBA_CHMELE_CHMELOVANI = 1;
const int SPOTREBA_KVASINEK_KVASENI = 1;

/* Timeouty */
const double TIME_VYNUCENI_SPILKY = 24;

/* Casy obsluh */
const double TIME_L_KLICENI = 60;
const double TIME_H_KLICENI = 80;

const double TIME_HVOZDENI = 1 DEN;

const double TIME_L_SROTOVANI = 1;
const double TIME_H_SROTOVANI = 2;
const double TIME_CISTENI_SROTOVNIKU = 15 MINUTA;

const double TIME_VYSTIRANI = 5.5;

const double TIME_L_SCEZENI = 2.5; 
const double TIME_H_SCEZENI = 3.5; 
const double TIME_CISTENI_KADE = 0.5; 

const double TIME_L_1_CHMELENI = 1;
const double TIME_H_1_CHMELENI = 1.5;
const double TIME_2_CHMELENI = 0.5;
const double TIME_CERPANI = 15 MINUTA;

const double TIME_L_ZACHLAZENI = 1.5;
const double TIME_H_ZACHLAZENI = 2;
const double TIME_PRIDANI_KVASINEK = 3 MINUTA;

const double TIME_1_HLAVNI_KVASENI = 7 DEN;
const double TIME_2_HLAVNI_KVASENI = 10 DEN;
const double TIME_3_HLAVNI_KVASENI = 12 DEN;

const double TIME_L_DOKVASENI = 14 DEN;
const double TIME_H_DOKVASENI = 1 MESIC;

const double TIME_FILTRACE = 1 DEN;

const double TIME_L_PASTERIZACE = 20 MINUTA;
const double TIME_H_PASTERIZACE = 30 MINUTA;

const double TIME_STACENI = 2;


/* -----=================== Cisteni kade/srotovniku ====================----- */

class CisteniKade : public Process
{
    void Behavior()
    {
        Wait(TIME_CISTENI_KADE);
        Leave(VystiraciKad);
    }
};

class CisteniSrotovniku : public Process
{
    void Behavior()
    {
        Wait(TIME_CISTENI_SROTOVNIKU);
        Leave(Srotovnik);
    }
};

/* -----======================== Proces Staceni ========================----- */

/**
 * Posledni faze vyrobniho procesu.
 */
class Staceni : public Process
{
    
private:
    
    int m_priorita;
    double m_startTime;
    
public:
    
    Staceni(int priorita, double startTime) : 
        Process(priorita), m_priorita(priorita), m_startTime(startTime)
    {
    }
    
    void Behavior()
    {
        Enter(Stacirna);
        Wait(TIME_STACENI); // Staceni do lahvy
        Leave(Stacirna);

        hotoveLitry[m_priorita] += VELIKOST_VARKY;
        pocetDokoncenychLitru += VELIKOST_VARKY;

#ifdef DEBUG
        cout << "++ Piva Dokoncena | P=" << m_priorita << " | SUM = " 
             << hotoveLitry[m_priorita] << endl;
#endif

        HistDokonceniVarek(Time - m_startTime);
    }
};

/* -----========================== Druha faze ==========================----- */

/**
 * Druha faze vyrobniho procesu. 
 * Proces vytvoren slouceni varek podle KAPACITA_SPILKY_VE_VARKACH.
 * 
 * Odkaleni a zchlazeni -> pridani kvasinek -> hlavni kvaseni -> dokvaseni ->
 *      -> fitrace -> pasterizace -> rozdeleni na staceci varky
 */
class KvaseniOsetreni : public Process
{
    
private:
    
    vector<int> m_priorVect;
    vector<double> m_timeVect;
    int m_priorita;
    
public:
    
    KvaseniOsetreni(vector<int> priorVect, vector<double> timeVect) : 
        m_priorVect(priorVect), m_timeVect(timeVect)
    {
        // Vysledna varka bude mit prioritu odpovidajici nejvetsi priorite
        // ze vsech sloucenych varek
        m_priorita = 0;
        
        
        int count = m_priorVect.size();
        for (int i = 0; i < count; i++)
        {
            if (m_priorVect[i] > m_priorita)
                m_priorita = m_priorVect[i];
        }
        
        Priority = m_priorita;

        
        double effect = static_cast<double>(count) / KAPACITA_SPILKY_VE_VARKACH;
        VyuzitiSpilek.add(effect);
        
#ifdef DEBUG
        cout << "Druha faze | Pocet sloucenych varek = " << count << " | Efektivita vyuziti spilek: " << effect << endl;
#endif
    }

    void Behavior()
    {
        // Odkaleni a zachlazeni
        Wait(Uniform(TIME_L_ZACHLAZENI, TIME_H_ZACHLAZENI)); 

        SkladKvasinek.get(this, SPOTREBA_KVASINEK_KVASENI);
        
        Wait(TIME_PRIDANI_KVASINEK);
        
        // Hlavni kvaseni
        double uniRand = Uniform(0.0,0.99);
        if (uniRand < 0.33)
        {
            Wait(TIME_1_HLAVNI_KVASENI);
        }
        else if (uniRand < 0.66)
        {
            Wait(TIME_2_HLAVNI_KVASENI);
        }
        else
        {
            Wait(TIME_3_HLAVNI_KVASENI);
        }
        
        // Uvolneni spilky a nastaveni skladu preSpilky
        Leave(Spilka);
        if (Spilka.Free() == 1 && PreSpilkaSklad.Free() == 0)
        {
            Enter(Spilka);
            PreSpilkaSklad.Leave(KAPACITA_SPILKY_VE_VARKACH);
        }

        // Dokvaseni v tancich
        Enter(DokvasujiciTanky);
        Wait(Uniform(TIME_L_DOKVASENI, TIME_H_DOKVASENI)); 
        Leave(DokvasujiciTanky);

        // Filtrace
        Enter(Filtry);
        Wait(TIME_FILTRACE); 
        Leave(Filtry);

        // Pasterizase
        Wait(Uniform(TIME_L_PASTERIZACE, TIME_H_PASTERIZACE)); 
        
        int count = m_priorVect.size();
        for (int i = 0; i < count; i++)
        {   // vytvoreni procesu staceni s prislusnou prioritou
            (new Staceni(m_priorVect[i], m_timeVect[i]))->Activate(); 
        }
    }
};

/* -----================= Timeout pri cekani na spilky =================----- */

class VynuceniSpilkyTimeout : public Process
{
    void Behavior();
};

/* -----====================== Priprava a vareni =======================----- */

/**
 * Prvni faze vyrobniho procesu.
 * 
 * Priprava varky -> kliceni -> hvozdeni -> srotovani -> vystirani -> scezeni ->
 *      -> vareni v kotly -> slouceni nekolika varek pro dalsi proces
 */
class PripravaVareni : public Process
{
    
private:
    
    int m_priorita;
    double m_startTime;
    
    VynuceniSpilkyTimeout *m_vynuceniSpilkyTimeout;
    
public:

    PripravaVareni(int priorita) : Process(priorita), m_priorita(priorita)
    {
//        Priority = priorita;
        m_vynuceniSpilkyTimeout = NULL;
    }

    int getPriority()
    {
        
        return m_priorita;
    }
    
    int getStartTime()
    {
        return m_startTime;
    }
    
    void cancelPojisTimeout()
    {
        if (m_vynuceniSpilkyTimeout != NULL)
        {
            m_vynuceniSpilkyTimeout->Cancel();
        }
    }

    void Behavior()
    {
        
#ifdef DEBUG
        cout << "Nova varka" << endl;
#endif
        
        Seize(PripravaVarky); 

        m_startTime = Time;
        
        // Cekani na pripravu surovin
        SkladVody.get(this,SPOTREBA_VODY_KLICENI);
        
        SkladJecmene.get(this,SPOTREBA_JECMENE_KLICENI);
        
        Release(PripravaVarky);
        
        // Kliceni
        Enter(Humna);
        Wait(Uniform(TIME_L_KLICENI, TIME_H_KLICENI));
        Leave(Humna);

        // Hvozdeni
        Enter(Hvozdy);
        Wait(TIME_HVOZDENI);
        Leave(Hvozdy);

        // Srotovani
        Enter(Srotovnik);
        Wait(Uniform(TIME_L_SROTOVANI, TIME_H_SROTOVANI));
        
        // Vytvoreni procesu cisteni srotovniku
        (new CisteniSrotovniku)->Activate(); 

        Enter(VystiraciKad);
        SkladVody.get(this,SPOTREBA_VODY_VYSTIRANI);
        Wait(TIME_VYSTIRANI); // Vystirani
        Wait(Uniform(TIME_L_SCEZENI, TIME_H_SCEZENI)); // Scezeni
        
        // Vytvoreni procesu cisteni kade
        (new CisteniKade)->Activate();

        // Cekani na pripravu chmele
        SkladChmele.get(this, SPOTREBA_CHMELE_CHMELOVANI);
        
        
        // Nastaveni skladu preSpilky
        if (Spilka.Free() > 0 && PreSpilkaSklad.Free() == 0)
        {
            Enter(Spilka);
            PreSpilkaSklad.Leave(KAPACITA_SPILKY_VE_VARKACH);
        }
        
        Enter(PreSpilkaSklad);

        Enter(VarnyKotel); // Cekani na uvolneni kotle
        
        SkladVody.get(this, SPOTREBA_VODY_CHMELENI);
        
        Wait(Uniform(TIME_L_1_CHMELENI, TIME_H_1_CHMELENI)); // 1. chmeleni
        Wait(TIME_2_CHMELENI); // 2. chmeleni
        Wait(TIME_CERPANI);
        
        Leave(VarnyKotel);
        
        SpilkyFronta.push_back(this);
        
        if (SpilkyFronta.size() == 1)
        { // Prvni varka ve fronte -> stara se o timeout
            // Timeout pro vynuceni zapoceti kvaseni ve spilce
            m_vynuceniSpilkyTimeout = new VynuceniSpilkyTimeout();
            m_vynuceniSpilkyTimeout->Activate(Time + TIME_VYNUCENI_SPILKY);

            // Cekani na varky, aby byla spilka vyuzita na plnou kapacitu
            WaitUntil(SpilkyFronta.size() >= KAPACITA_SPILKY_VE_VARKACH);

            // Zruseni timeoutu
            m_vynuceniSpilkyTimeout->Cancel();

            // Ulozeni vsech priorit a casu varek, ktere budou slouceny
            vector<int> priorVect;
            vector<double> timeVect;
            
            // Tuto varku nechci ukoncit, proto je mimo for cyklus
            priorVect.push_back(m_priorita);
            timeVect.push_back(m_startTime);
            SpilkyFronta.pop_front();

            // Ulozeni a odstraneni dalsich varek, ktere cekaji ve fronte
            for (unsigned int i = 1; i < KAPACITA_SPILKY_VE_VARKACH; i++)
            {
                // Vyber druhe varky ve fronte
                PripravaVareni *auxVarka = SpilkyFronta.front();
                SpilkyFronta.pop_front();

                // Ulozeni a uvolneni varky
                priorVect.push_back(auxVarka->getPriority());
                timeVect.push_back(auxVarka->getStartTime());
                auxVarka->Cancel();
            }

            // Vytvoreni procesu druhe faze, ktera vznikla sloucenim cekajicich varek
            (new KvaseniOsetreni(priorVect, timeVect))->Activate();
        }
        else // Ve fronte uz ceka varka, ktera se stara o timeout
        {
            // Cekani na vyzvednuti od prvni varky, ktere vyprsi timeout
            // nebo prijde posledni potreba varka
            Passivate();
        }
    }
};

/**
 * Vynuceni zapoceti kvaseni ve spilce.
 */
void VynuceniSpilkyTimeout::Behavior()
{
#ifdef DEBUG
    cout << "** Vynuceni pouziti spilky!" << std::endl;
#endif

    // Odebrani mist v preSpilce
    Enter(PreSpilkaSklad, PreSpilkaSklad.Free());
    
    unsigned int frontaSizeCurrent = SpilkyFronta.size();
    
    // V kotly se vari varka, cekam na ni
    if (VarnyKotel.Capacity() != VarnyKotel.Free())
    { 
#ifdef DEBUG
        cout << "    V kotli se vari varka, cekam na tuto varku..." << endl;
#endif
        WaitUntil(SpilkyFronta.size() == (frontaSizeCurrent + 1));
    }
    
    // Ulozeni vsech priorit a casu varek, ktere budou slouceny
    vector<int> priorVect;
    vector<double> timeVect;

    for (unsigned int i = 0; i < KAPACITA_SPILKY_VE_VARKACH; i++)
    {
        if (SpilkyFronta.empty())
            break;
        
        // Vyber prvni varky ve fronte
        PripravaVareni *auxVarka = SpilkyFronta.front();
        SpilkyFronta.pop_front();

        
        // Ulozeni priority a uvolneni varky
        priorVect.push_back(auxVarka->getPriority());
        timeVect.push_back(auxVarka->getStartTime());
        auxVarka->Cancel();
    }
    
    // Vytvoreni procesu druhe faze, ktera vznikla sloucenim cekajicich varek
    (new KvaseniOsetreni(priorVect, timeVect))->Activate();
    
    Cancel();
}

/* -----========================= Objednavka ===========================----- */

/**
 * Proces objednavky.
 */
class Objednavka : public Process
{
    void Behavior()
    {
        // Priorita objednavky
        int priorita;
        
        // Vyber velikosti objednavky
        int velikostObjednavky;
        if (MIN_POCET_VAREK_V_OBJEDNAVCE != MAX_POCET_VAREK_V_OBJEDNAVCE)
        {
            velikostObjednavky = (rand() % (MAX_POCET_VAREK_V_OBJEDNAVCE - MIN_POCET_VAREK_V_OBJEDNAVCE)) + MIN_POCET_VAREK_V_OBJEDNAVCE;
        }
        else
        {
            velikostObjednavky = MIN_POCET_VAREK_V_OBJEDNAVCE ;
        }
        
        // Velikost objednavky v litrech
        int pocetLitru = VELIKOST_VARKY * velikostObjednavky;
        
        // Cas zapoceti objednavky
        double startTime = Time;
        
        // Prioritizace objednavky
        if (Uniform(0.0,1.0) > 0.2)
        {
            priorita = NORMALNI_PRIORITA;
        }
        else
        {
            priorita = VYSOKA_PRIORITA;
        }

        // Generovani prislusneho poctu varek
        for (int i = 0; i < velikostObjednavky; i++)
        {
            (new PripravaVareni(priorita))->Activate();
        }

#ifdef DEBUG
        static int id = 1;
        int myid = id++;
        cout << "Nova objednavka | T=" << Time << " | P=" << priorita 
             << " | ID=" << myid << " | LITRU=" << pocetLitru << endl;
#endif
        
        // Cekani na dokonceni objednavky v prislusne fronte
        Seize(CekaniNaDokonceniObjednavky[priorita]);
        
        WaitUntil(hotoveLitry[priorita] >= pocetLitru);
        
        hotoveLitry[priorita] -= pocetLitru;
        
        Release(CekaniNaDokonceniObjednavky[priorita]);

#ifdef DEBUG
        cout << "Objednavka zpracovana a dokoncena | T=" << Time << "_" 
             << Time - startTime << " | P=" << priorita << " | ID=" << myid << endl;
#endif
        
        // Ulozeni doby zpracovani objednavky
        HistDokonceniObjednavek(Time - startTime);
    }
};

/* -----===================== Generator objednavek =====================----- */

class GeneratorObjednavek : public Event
{
    void Behavior()
    {
        (new Objednavka)->Activate();

        if (ROZLOZENI_PRICHODU_OBJEDNAVEK == 'e')
        {
            Activate(Time + Exponential(TIME_PRICHOD_OBJEDNAVEK));
        }
        else // rovnomerne
        {
            Activate(Time + TIME_PRICHOD_OBJEDNAVEK);
        }
    }
};

/* -----===================== Generator objednavek =====================----- */

class InicializaceSkladu : public Process
{
    void Behavior()
    {
        // Vyprazdneni preSpilky
        Enter(PreSpilkaSklad,PreSpilkaSklad.Free());
        
        /* Sklady surovin */
        SkladVody.activate();
        SkladJecmene.activate();
        SkladKvasinek.activate();
        SkladChmele.activate();
    }
};

/* -----========================== Statistiky ==========================----- */

void printStat(Store *storeUsage, Store *storeTime)
{
    
    if (storeTime == NULL)
    {
        storeTime = storeUsage;
    }
    
    double avgUse = storeUsage->tstat.MeanValue() / storeUsage->Capacity() * 100;
    double avgWaitTime;

    if (storeTime->Q->StatDT.Min() != storeTime->Q->StatDT.Max())
    {
        avgWaitTime = storeTime->Q->StatDT.MeanValue();
    }
    else
    {
        avgWaitTime = 0;
    }
    double maxWaitTime = storeTime->Q->StatDT.Max();
    double avgQueueLen = storeTime->Q->StatN.MeanValue();
    double maxQueueLen = storeTime->Q->StatN.Max();
    
    printf (" %4.1f%% | %9.2f | %9.2f | %10.2f | %10.2f |\n", 
        avgUse, avgWaitTime, maxWaitTime , avgQueueLen, maxQueueLen);
}

void printStat(Store *store)
{
    printStat(store, store);
}

void printStats()
{
    cout << "+-------------------+-------+-----------+-----------+------------+------------+" << endl; 
    cout << "| Store usage       | Avg.  |    Avg.   |    Max.   | Avg. queue | Max. queue |" << endl; 
    cout << "| time unit = hour  | usage | wait time | wait time |   length   |   length   |" << endl; 
    cout << "+-------------------+-------+-----------+-----------+------------+------------+" << endl; 
    cout << "| " << Humna.Name() << "             |"; printStat(&Humna);
    cout << "| " << Hvozdy.Name() << "            |"; printStat(&Hvozdy);
    cout << "| " << Srotovnik.Name() << "           |"; printStat(&Srotovnik);
    cout << "| " << VystiraciKad.Name() << "     |"; printStat(&VystiraciKad);
    cout << "| " << VarnyKotel.Name() << "       |"; printStat(&VarnyKotel);
    cout << "| " << Spilka.Name() << "    |"; printStat(&Spilka,&PreSpilkaSklad);
    cout << "| " << DokvasujiciTanky.Name() << " |"; printStat(&DokvasujiciTanky);
    cout << "| " << Filtry.Name() << "            |"; printStat(&Filtry);
    cout << "| " << Stacirna.Name() << "          |"; printStat(&Stacirna);
    cout << "+-------------------+-------+-----------+-----------+------------+------------+" << endl << endl; 
    printf("Efektivita vyuziti spilek: %2.1f%%\n",VyuzitiSpilek.get()*100);
    printf("Pocet dokoncenych litru piva: %d\n",pocetDokoncenychLitru);
}

void printParams()
{
    cout << "Parametry:" << endl 
         << "   Delka simulace: " << TIME_DELKA_SIMULACE << " hodin" << endl
         << "   Rozlozeni prichodu objednavek: "
         << ((ROZLOZENI_PRICHODU_OBJEDNAVEK == 'e') ? "exponencialni" : "rovnomerne") << endl
         << "   Cas prichod objednavek: " << TIME_PRICHOD_OBJEDNAVEK << " hodin" << endl
         << "   Minimalni pocet varek v objednavce: " << MIN_POCET_VAREK_V_OBJEDNAVCE << endl
         << "   Maximalni pocet varek v objednavce: " << MAX_POCET_VAREK_V_OBJEDNAVCE << endl  << endl;
}

/* -----===================== Zpracovani parametru =====================----- */

void zpracovaniParametru(int argc, char** argv)
{
    if (argc >= 2)
    {
        if (argv[1][0] == '-' && argv[1][1] == 'h')
        {
            cout << HELP;  exit(0);
        }
            
		if(argv[1][0] != 'd') // delka simulace
		{
            TIME_DELKA_SIMULACE = std::atol(argv[1]);
            
            if (TIME_DELKA_SIMULACE <= 0)
            {
                cout << HELP; 
                exit(0);
            }
		}
	}
	if(argc >= 3) 
	{
		if(argv[2][0] != 'd') // rozlozeni prichodu objednavek
        {
			ROZLOZENI_PRICHODU_OBJEDNAVEK = argv[2][0];
        }
	}
	if(argc >= 4) // cas prichodu objednavek
	{
		if(argv[3][0] != 'd')
        {
            TIME_PRICHOD_OBJEDNAVEK = std::atol(argv[3]);
            
            if (TIME_PRICHOD_OBJEDNAVEK <= 0)
            {
                cout << HELP; 
                exit(0);
            }
        }
	}
	if(argc >=5) // minimalni pocet varek v objednavce
	{
		if(argv[4][0] != 'd')
        {
            MIN_POCET_VAREK_V_OBJEDNAVCE = std::atol(argv[4]);
            
            if (MIN_POCET_VAREK_V_OBJEDNAVCE <= 0)
            {
                cout << HELP; 
                exit(0);
            }
            
        }
	}
	if(argc >= 6) // maximalni pocet varek v objednavce
	{
		if(argv[5][0] != 'd')
        {
            MAX_POCET_VAREK_V_OBJEDNAVCE = std::atol(argv[5]);
            
            if (MAX_POCET_VAREK_V_OBJEDNAVCE <= 0)
            {
                cout << HELP; 
                exit(0);
            }
        }
	}
    
    if (MAX_POCET_VAREK_V_OBJEDNAVCE < MIN_POCET_VAREK_V_OBJEDNAVCE)
    {
        cerr << "Chybne zadany pocet objednavek!" << endl;
        cout << HELP; 
        exit(1);
    }
}

/* -----============================= MAIN =============================----- */

int main(int argc, char** argv)
{
    srand(time(0));
    RandomSeed(time(0));
    
    zpracovaniParametru(argc,argv);
    
    /* Inicializace */
    Init(0, TIME_DELKA_SIMULACE);
    (new InicializaceSkladu)->Activate();
    
    /* Generator objednavek */
    (new GeneratorObjednavek)->Activate(); 
    
    /* Spusteni */
    Run();
    
    /* Vypisy */
    printParams();
    
    HistDokonceniObjednavek.Output();
    HistDokonceniVarek.Output(); 
    
    printStats();
    
    return 0;
}
