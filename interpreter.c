#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#define ROZMIAR_STOSU  4000
#define LICZBA_INSTRUKCJI 3000
#define ROZMIAR_PAMIECI 10000
#define MAX_DLUGOSC_ETYKIET 2000
#define KOD_CYFRY 48	
#define MAX_LICZBA_ETYKIET 1000
#define PRZESUNIECIE_PAMIEC 5000

struct lista_etykiet 
{
	/*
	Kolejne etykiety zapisujemy obok siebie w kolejne_etykiety[]
	Przy wczytywaniu nowej etykiety dopisujemy ja na koniec kolejne_etykiety
	i sprawdzamy, czy juz gdzies jest zapisana w kolejne_etykiety, jesli tak 
	to dubel_obecnej_etykiety przypisujemy miejsce jej pierwszego wystapienia w kolejne_etykiety[]
	Jesli etykiety nie ma w kolejne_etykiety[] to dodajemy ja do listy 
	( co oznacza zwiekszenie liczby_etykiet, dopisanie poczatku nowej etykiety do poczatki_etykiet i 
	dodanie do ostatni_znak_na_liscie_etykiet dlugosci nowej etykiety
	
	poczatki_etykiet[i] mowi o tym, w ktorej komorce kolejne_etykiety znajdziemy kod pierwszego
	znaku i-tej etykiety (etykiety numerujemy od zera) 
	*/
	int kolejne_etykiety[ 2*MAX_DLUGOSC_ETYKIET ];
	int ostatni_znak_w_liscie_etykiet; 
	int poczatki_etykiet[ MAX_LICZBA_ETYKIET ];
	int liczba_etykiet;
	int dubel_obecnej_etykiety; 
} etykiety;


enum rodzaje_instrukcji { DEFINICJA, ODEJMOWANIE, SKOK, WYWOLANIE, POWROT, CZYTANIE, PISANIE, KONIEC_INSTRUKCJI };


void inicjalizuj_pamiec( int pamiec [] ) 
// ustawia poczatkowe wartosc pamieci
{
	for ( int i = 0; i <= 4999; i++ ) 
	{
		pamiec[i] = -1 + (PRZESUNIECIE_PAMIEC - i);
	}
	for ( int i = 5000; i < ROZMIAR_PAMIECI; i++)
	{
		pamiec[i] = -1 - (i - PRZESUNIECIE_PAMIEC);
	}	
}


void omin_separatory() 
// omija separatory we wczytywanym tekscie - spacje, tabulatory, znaki konca wiersza i '|'
{
	int wczytany_znak;
	wczytany_znak = getchar();
	while ( ( wczytany_znak == 9 ) || ( wczytany_znak == 10 ) || ( wczytany_znak == 124 ) || ( wczytany_znak == 32 ) )
	{
		wczytany_znak = getchar();
	}
	ungetc( wczytany_znak, stdin );	
}


void inicjalizuj_stos( int stos_powrotu[] )
{
	stos_powrotu[0] = 1;	
	/* stos_powrotu[0] wskazuje na pierwsze wolne miejsce na stosie
	elementy stosu zapisujemy na wspolrzednych 1,2,...,ROZMIAR_STOSU
	elementami stosu sa numery instrukcji	
	*/
}


int ostatnia_na_stosie( int stos_powrotu[] )
// nie sprawdzamy tutaj, czy stos jest pusty, bo wywolanie funkcji ostatnia_na_sosie bedzie zawsze poprzedzone wywolaniem czy_stos_pusty
{	
	return stos_powrotu[ stos_powrotu[0] - 1 ];	
}


void dodaj_do_stosu( int stos_powrotu[], int ktora_instrukcja )
{
	stos_powrotu[ stos_powrotu[0] ] = ktora_instrukcja;
	stos_powrotu[0]++;
}


void zdejmij_ze_stosu( int stos_powrotu[] )
{
	if (stos_powrotu[0] > 1) 
	{	
		stos_powrotu[0]--;
	}
}


bool czy_stos_pusty( int stos_powrotu[] )
{
	if ( stos_powrotu[0] == 1 ) 
		return true;
	else
		return false;
}


int zwroc_wartosc_adresu( int pamiec [], int adres ) 
// wykonuje operacje *adres
{
	if ( ( adres <= 4999 ) && ( adres >= -5000 ) )
	// tylko komorki od -5000 do 4999 moga zmieniac swoja wartosc w trakcie dzialania programu, pozostale bede stale mialy wartosc -1 - adres
	{
		return pamiec[ adres + PRZESUNIECIE_PAMIEC ];
	}
	else 
		return -1 -adres;	
}


int znajdz_definicje_etykiety( int rekord_instrukcji[][ LICZBA_INSTRUKCJI ], enum rodzaje_instrukcji typ_instrukcji[], int numer_instrukcji )
// znajduje miejsce w kodzie, w ktorym pojawia sie definicja etykiety z instrukcji o numerze numer_instrukcji
{
	int numer_instrukcji_z_definicja;
	bool znalezlismy_definicje = false;
	int licznik = 0;
	while ( !znalezlismy_definicje )
	{
		if ( ( typ_instrukcji[ licznik ] == DEFINICJA) && ( rekord_instrukcji[1][ licznik ] == rekord_instrukcji[1][ numer_instrukcji ] ) )
		{
			znalezlismy_definicje = true;
			numer_instrukcji_z_definicja = licznik;
		}		
		licznik++;		
	}	
	
	return numer_instrukcji_z_definicja;	
}


bool znak_jest_wyrazeniem_arytmetycznym( int wczytany_znak ) 
//czy wczytany znak jest cyfra,  '+' lub '-'
{
	if ( ( wczytany_znak == '+' ) || ( wczytany_znak == '-' ) || ( isdigit( wczytany_znak ) != 0 ) )
		return true;
	else 
		return false;	
}


void wczytaj_adres( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ],  int licznik_instrukcji, int wczytany_znak , int do_ktorego_wiersza )
// wczytuje liczbe Adres do rekord_instrukcji[do_ktorego_wiersza][licznik_instrukcji]
{
	bool czy_ujemna = ( wczytany_znak == '-' );
	int wczytany_adres = 0;
	if ( isdigit(wczytany_znak) == 0 )
	// na poczatku moze pojawic sie jeden  plus lub jeden minus
	{
		wczytany_znak = getchar();
	}	
	while ( isdigit( wczytany_znak ) != 0 )
	// wczytujemy, dopoki natrafiamy na cyfry
	{
		wczytany_adres = 10*wczytany_adres + ( wczytany_znak - KOD_CYFRY );
		// konwersja liczby w zapisie dziesietnym na wartosc liczbowa
		wczytany_znak = getchar();
	}
	ungetc( wczytany_znak, stdin );
	
	if ( czy_ujemna )
	{
		rekord_instrukcji[ do_ktorego_wiersza ][ licznik_instrukcji ] = -wczytany_adres;
	}
	else
	{
		rekord_instrukcji[ do_ktorego_wiersza ][ licznik_instrukcji ] = wczytany_adres;
	}	
}


bool cyfra_litera_lub_podkreslenie( int wczytany_znak )
// sprawdza, czy wczytany znak jest znakiem, ktory moze wchodzic w sklad nazwy etykiety
{	
	if ( ( isdigit( wczytany_znak ) != 0 ) || ( wczytany_znak == '_' ) || ( isalpha( wczytany_znak ) != 0 ) )
	{
		return true;
	}
	else
		return false;	
}


bool czy_te_same_etykiety( int dlugosc_etykiety, int numer_etykiety )
// sprawdza, czy etykieta dopisana tymczasowo do konca etykiety.kolejne_etykiety jest taka sama jak etykieta o numerze numer_etykiety
{
	bool te_same = true;
	if ( ( numer_etykiety < etykiety.liczba_etykiet ) &&( dlugosc_etykiety != etykiety.poczatki_etykiet[ numer_etykiety ] - etykiety.poczatki_etykiet[ numer_etykiety - 1 ] ) )
	// najpierw sprawdzamy, czy porownywane etykiety maja te sama dlugosc - osobno musimy rozwazyc przypadek kiedy etykieta, z ktora porownujemy jest na koncu listy etykiet
	{
		te_same = false;
	}
	else if ( ( numer_etykiety == etykiety.liczba_etykiet ) && ( dlugosc_etykiety != etykiety.ostatni_znak_w_liscie_etykiet - etykiety.poczatki_etykiet[ numer_etykiety - 1 ] + 1 ) )
	// przypadek, gdy etykieta, z ktora porownujemy lezy na koncu listy etykiet
	{
		te_same = false;
	}
	else
	// porownywane etykiety maja te sama dlugosc
	{	
		for ( int i = 0; i <= dlugosc_etykiety - 1 ; i++ )
		{
			if ( etykiety.kolejne_etykiety[ etykiety.ostatni_znak_w_liscie_etykiet + i + 1 ] != etykiety.kolejne_etykiety[ etykiety.poczatki_etykiet[ numer_etykiety - 1 ] + i ] )
			{
				te_same = false;
			}				
		}
	}
	if ( te_same )
	{
		etykiety.dubel_obecnej_etykiety = numer_etykiety;
	}
	
	return te_same;
}


bool etykieta_juz_byla( int dlugosc_etykiety )
// sprawdza czy dana etykieta juz jest wpisana do etykiety.kolejne_etykiety[]
{
	bool juz_byla = false;
	int numer_etykiety = 1;
	while ( (numer_etykiety <= etykiety.liczba_etykiet ) && ( !juz_byla ) )
	{
		juz_byla = czy_te_same_etykiety( dlugosc_etykiety, numer_etykiety );
		numer_etykiety++;
	}
	
	return juz_byla;
}


void czytaj_etykiete( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ],  int licznik_instrukcji, int wczytany_znak )
/* dopisujemy czytana etykiete na koniec etykiety.kolejne_etykiety i sprawdzamy, czy etykieta pojawila sie juz wczesniej w jakiejs instrukcji,
   co jest rownowazne temu, ze jest juz gdzies wczesniej wpisana w etykiety.kolejne_etykiety */
{
	int dlugosc_nowej_etykiety = 0;
	while ( cyfra_litera_lub_podkreslenie( wczytany_znak ) )
	{
		dlugosc_nowej_etykiety++;
		etykiety.kolejne_etykiety[ etykiety.ostatni_znak_w_liscie_etykiet + dlugosc_nowej_etykiety ] = wczytany_znak;
		wczytany_znak = getchar();		
	}
	ungetc( wczytany_znak, stdin );
	if ( !etykieta_juz_byla( dlugosc_nowej_etykiety ) )
	{
		etykiety.ostatni_znak_w_liscie_etykiet = etykiety.ostatni_znak_w_liscie_etykiet + dlugosc_nowej_etykiety;
		etykiety.liczba_etykiet++;
		etykiety.poczatki_etykiet[ etykiety.liczba_etykiet - 1 ] = etykiety.ostatni_znak_w_liscie_etykiet - dlugosc_nowej_etykiety + 1;
		rekord_instrukcji[1][ licznik_instrukcji ] = etykiety.liczba_etykiet; 
	}
	else
	// etykieta juz pojawila sie w kodzie
	{		
		rekord_instrukcji[1][ licznik_instrukcji ] = etykiety.dubel_obecnej_etykiety;		
	}
}


void pierwsza_skladnia_jest_adresem( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ], enum rodzaje_instrukcji typ_instrukcji[], int licznik_instrukcji, int wczytany_znak )
// uzywamy w sytuacji, gdy pierwszy element wczytywanej instrukcji byl adresem komorki pamieci 
{
  
	if ( znak_jest_wyrazeniem_arytmetycznym( wczytany_znak ) )		
	{
		wczytaj_adres( rekord_instrukcji,  licznik_instrukcji, wczytany_znak, 1);
		typ_instrukcji[ licznik_instrukcji ] = ODEJMOWANIE;
	}
	else if ( wczytany_znak == '^' )
	{
		typ_instrukcji[ licznik_instrukcji ] = PISANIE;
	}
	else
	{
		czytaj_etykiete( rekord_instrukcji, licznik_instrukcji, wczytany_znak );
		typ_instrukcji[ licznik_instrukcji ] = SKOK;
	}   
}


bool wczytaj_pojedyncza_instrukcje ( int  rekord_instrukcji[][LICZBA_INSTRUKCJI ], enum rodzaje_instrukcji typ_instrukcji[], int licznik_instrukcji)
/* 
wczytuje pojedyncza instrukcje do rekord_instrukcji[][licznik_instrukcji] i zapisuje jej typ w typ_instrukcji[licznik_instrukcji]
zwracany bool informuje o tym, czy zostaly jeszcze jakies instrukcje do wczytania
*/
{
	int wczytany_znak;
	omin_separatory();		
	wczytany_znak = getchar();
	if ( znak_jest_wyrazeniem_arytmetycznym ( wczytany_znak ) )
	{
		wczytaj_adres( rekord_instrukcji,  licznik_instrukcji, wczytany_znak, 0);
		omin_separatory();
		wczytany_znak = getchar();
		pierwsza_skladnia_jest_adresem( rekord_instrukcji, typ_instrukcji, licznik_instrukcji, wczytany_znak );
	}
	else 
	{
	    switch( wczytany_znak )
	    {	      
	    	case '^':
	    	{		
			omin_separatory();
			wczytany_znak = getchar();
			wczytaj_adres( rekord_instrukcji,  licznik_instrukcji, wczytany_znak, 0);
			typ_instrukcji[ licznik_instrukcji ] = CZYTANIE;
	    	} 
		break;		
	    	case ';':
	    	{
	    		typ_instrukcji[ licznik_instrukcji ] = POWROT;
		} 
		break;
		case ':':
		{
			typ_instrukcji[ licznik_instrukcji ] = DEFINICJA;
			omin_separatory();
			wczytany_znak = getchar();
			czytaj_etykiete( rekord_instrukcji, licznik_instrukcji, wczytany_znak );				
		} 
		break;
	    	default:
	    	// jesli zaden z powyzszych case'ow nie zaszedl, to musimy miec do czynienia z wywolaniem (bo wiemy, ze kod jest poprawny)
	    	{
			typ_instrukcji[ licznik_instrukcji ] = WYWOLANIE;
			czytaj_etykiete( rekord_instrukcji, licznik_instrukcji, wczytany_znak );				
	    	}	 
		break;
	    }	  	  	   	  	  		  
	 }
	  	
	omin_separatory();
	wczytany_znak = getchar();
	if ( ( wczytany_znak == EOF ) || ( wczytany_znak == '&' ) )
	// sprawdzamy, czy zostaly jakies instrukcje do wczytania
	{
		return false;
	}
	else
	{
		ungetc( wczytany_znak, stdin);
		return true;
	}
}


void czytaj_instrukcje( int  rekord_instrukcji[][LICZBA_INSTRUKCJI ], enum rodzaje_instrukcji typ_instrukcji[] ) 
{
	bool zostaly_jakies_instrukcje = true ;
	int licznik_instrukcji = 0;
	omin_separatory();	
	int wczytany_znak;
	wczytany_znak = getchar();
	if ( ( wczytany_znak == EOF ) || ( wczytany_znak == '&' ) )
	// sprawdzamy najpierw, czy w ogole dostalismy jakies instrukcje
	{
		zostaly_jakies_instrukcje = false ;
	}
	ungetc( wczytany_znak, stdin );
	while ( zostaly_jakies_instrukcje )
	{
		zostaly_jakies_instrukcje = wczytaj_pojedyncza_instrukcje( rekord_instrukcji, typ_instrukcji, licznik_instrukcji );
		licznik_instrukcji++;		
	}
	
	typ_instrukcji[ licznik_instrukcji ] = KONIEC_INSTRUKCJI; 
	// po ostatniej wczytanej instrukcji dopisujemy straznika, zeby wiedziec, kiedy koncza sie instrukcje
}


int instrukcja_odejmowania( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ], int kolejna_instrukcja, int pamiec[] )
{
	pamiec[ zwroc_wartosc_adresu( pamiec, rekord_instrukcji[0][ kolejna_instrukcja ] ) + PRZESUNIECIE_PAMIEC] = zwroc_wartosc_adresu( pamiec, zwroc_wartosc_adresu( pamiec, rekord_instrukcji[0][ kolejna_instrukcja ] ) ) - zwroc_wartosc_adresu( pamiec, zwroc_wartosc_adresu( pamiec, rekord_instrukcji[1][ kolejna_instrukcja ] ) );
		
	return kolejna_instrukcja + 1;
}


int instrukcja_wywolania( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ], enum rodzaje_instrukcji typ_instrukcji[], int kolejna_instrukcja, int stos_powrotu[] )
{
	dodaj_do_stosu( stos_powrotu, kolejna_instrukcja + 1 ); 
		
	return  znajdz_definicje_etykiety( rekord_instrukcji, typ_instrukcji, kolejna_instrukcja) + 1;
}


int instrukcja_czytania( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ], int kolejna_instrukcja, int pamiec[] )
{
	int wczytana_wartosc;
	wczytana_wartosc = getchar(); 
	
	if ( wczytana_wartosc != EOF )
	{		
		pamiec[ zwroc_wartosc_adresu( pamiec, rekord_instrukcji[0][ kolejna_instrukcja ] ) + PRZESUNIECIE_PAMIEC] = wczytana_wartosc;	
	}
	else		
		pamiec[ zwroc_wartosc_adresu( pamiec, rekord_instrukcji[0][ kolejna_instrukcja ] ) + PRZESUNIECIE_PAMIEC] = -1;
			
	return kolejna_instrukcja + 1;	
}


int instrukcja_pisania( int  rekord_instrukcji[][LICZBA_INSTRUKCJI], int kolejna_instrukcja, int pamiec[] )
{
	putchar( zwroc_wartosc_adresu( pamiec, zwroc_wartosc_adresu( pamiec, rekord_instrukcji[0][ kolejna_instrukcja ]) ) );
	
	return kolejna_instrukcja + 1;	
}


int instrukcja_skoku( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ], enum rodzaje_instrukcji typ_instrukcji[], int kolejna_instrukcja, int pamiec[] )
{
	if ( zwroc_wartosc_adresu( pamiec, zwroc_wartosc_adresu( pamiec, rekord_instrukcji[0][ kolejna_instrukcja ] ) ) > 0 )
	{		
		return znajdz_definicje_etykiety( rekord_instrukcji, typ_instrukcji, kolejna_instrukcja ) + 1;
	}
	else 	
		return kolejna_instrukcja + 1;
}


int instrukcja_powrotu( int stos_powrotu[], bool *koniec_programu )
{
	int ostatnia_instrukcja = 0;
	if ( czy_stos_pusty(stos_powrotu) )
	{
		*koniec_programu = true;	
	}
	else
	{
		ostatnia_instrukcja = ostatnia_na_stosie( stos_powrotu );	
		zdejmij_ze_stosu( stos_powrotu );		
	}
		
	return ostatnia_instrukcja;	
}


int wykonaj_instrukcje( int  rekord_instrukcji[][ LICZBA_INSTRUKCJI ], enum rodzaje_instrukcji typ_instrukcji [], int kolejna_instrukcja, int stos_powrotu[], int pamiec[], bool *koniec_programu )
// Kazda instrukcja zwraca numer instrukcji, ktora ma zostac po niej wykonana 
{	
	switch ( typ_instrukcji[kolejna_instrukcja] )
	{	
	case DEFINICJA:		
		return kolejna_instrukcja + 1;
		break;
	case ODEJMOWANIE:	
		return instrukcja_odejmowania( rekord_instrukcji, kolejna_instrukcja, pamiec );
		break;
	case SKOK:		
		return instrukcja_skoku( rekord_instrukcji, typ_instrukcji, kolejna_instrukcja, pamiec );
		break;
	case WYWOLANIE:	
		return instrukcja_wywolania( rekord_instrukcji, typ_instrukcji, kolejna_instrukcja, stos_powrotu );
		break;
	case POWROT:		
		return instrukcja_powrotu( stos_powrotu, koniec_programu );
		break;
	case CZYTANIE:	
		return instrukcja_czytania( rekord_instrukcji, kolejna_instrukcja, pamiec );
		break;
	case PISANIE:	
		return instrukcja_pisania( rekord_instrukcji, kolejna_instrukcja ,pamiec );
		break;	
	default:
	  	return kolejna_instrukcja;
		break;
	}	
}


void program()
{	

	int stos_powrotu[ ROZMIAR_STOSU + 1 ];
	inicjalizuj_stos( stos_powrotu );

	int rekord_instrukcji[2][ LICZBA_INSTRUKCJI ]; 
	/*
	Sposob kodowania kolejnych instrukcji w tablicach rekord_instrukcji, typ_instrukcji, etykiety.kolejne_etykiety:

	w typ_instrukcji[i] zapisujemy rodzaj i-tej instrukcji wedlug typu enum rodzaje_instrukcji

	instrukcje dzielimy na 2 argumentowe ( odejmowanie, skok), 1 argumentowe (czytanie, pisanie, definicja, wywolanie), zero argumentowe( powrot );

	w przypadku zero argumentowych instrukcji wartosci rekord_instrukcji[0][i];rekord_instrukcji[1][i]
	pozostaja nieokreslone

	w przypadku dwuargumentowych do  rekord_instrukcji[0][i] zapisujemy pierwszy argument, a do rekord_instrukcji[1][i] drugi

	jesli argumentem jest Adres to zapisujemy jego wartosc liczbowa
	jesli argumentem jest etykieta, to zapisujemy numer jej wystapienia w liscie etykiety.kolejne_etykiety

	Dla instrukcji jednoargumentowych wpisujemy informacje o argumencie tylko do jednego wiersza rekord_instrukcji[][i] (drugi pozostaje nieokreslony)
	przy czym przyjmujemy konwencje, ze etykiety sa wpisywane zawsze do drugiego wiersza, a adresy do pierwszego 

	Na przyklad dla instrukcji ( o numerze i) 10^ zapiszemy rekord_instrukcji[0][i] = 10;
	Na przyklad dla instrukcji ( o numerze i) ^10 zapiszemy rekord_instrukcji[0][i] = 10;
	dla instrukcji (o numerze i) 10 nazwa_etykiety zapiszemy rekord_instrukcji[0][i] = 10; rekord_instrukcji[1][i] = numer wystapienia "nazwa_etykiety" w kolejne_etykiety
	dla instrukcji nazwa_etykiety zapiszemy  rekord_instrukcji[1][i] = numer wystapienia "nazwa_etykiety" w kolejne_etykiety
	dla instrukcji :nazwa_etykiety zapiszemy  rekord_instrukcji[1][i] = numer wystapienia "nazwa_etykiety" w kolejne_etykiety

	INSTRUKCJE NUMERUJEMY OD 0!
	*/
	enum rodzaje_instrukcji typ_instrukcji[ LICZBA_INSTRUKCJI + 1 ];

	int pamiec[ ROZMIAR_PAMIECI ];
	inicjalizuj_pamiec( pamiec );

	int kolejna_instrukcja = 0;

	bool *koniec_programu;
	koniec_programu = (bool*) malloc(sizeof(bool));
	*koniec_programu = false;

	etykiety.ostatni_znak_w_liscie_etykiet = - 1;
	etykiety.liczba_etykiet = 0;

	czytaj_instrukcje( rekord_instrukcji, typ_instrukcji );

	while ( ( typ_instrukcji [kolejna_instrukcja] != KONIEC_INSTRUKCJI) && ( !*koniec_programu) ) 
	// wykonujemy  instrukcje, dopoki nie skoncza sie instrukcje badz nie dostaniemy instrukcji powrotu na pustym stosie
	{	
		kolejna_instrukcja = wykonaj_instrukcje( rekord_instrukcji, typ_instrukcji, kolejna_instrukcja, stos_powrotu, pamiec, koniec_programu );	
	}	

	free( koniec_programu );
}


int main(void)
{	
	program();
	
	return 0;
}
