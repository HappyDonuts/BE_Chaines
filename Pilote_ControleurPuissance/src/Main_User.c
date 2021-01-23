
/*
	!!!! NB : ALIMENTER LA CARTE AVANT DE CONNECTER L'USB !!!

*/


/*
STRUCTURE DES FICHIERS

COUCHE APPLI : 
Main_User.c : programme principal à modifier. Par défaut hacheur sur entrée +/-10V, sortie 1 PWM

COUCHE SERVICE :
Toolbox_NRJ.c : contient toutes les fonctions utilisables, qui "parlent" à un électrotechnicien
Table_Cosinus_Tri.c : contient 3 tables de cosinus déphasés de 120°, 36 pts (10° de résolution)
Table_Cosinus_Tri_120pts.c : idem mais 120 pts par cosinus

COUCHE DRIVER :
clock.c : contient LA fonction Clock_Configure() qui prépare le STM32. Lancée automatiquement à l'init IO
lib : bibliothèque qui gère les périphériques du STM : Drivers_STM32F103_107_Jan_2015_b
*/



/*
VOIR LE FICHIER TOOLBOX_NRJ.H POUR TOUTES INFOS SUR LES E/S
*/




#include "ToolBox_NRJ_v3.h"
#include "Table_Cosinus_Tri.h"


//=================================================================================================================
// 					USER DEFINE
//=================================================================================================================

// Choix de la fréquence PWM (en kHz)
#define FPWM_Khz 20.0
// Choix de la fréquence d'échantillonnage de l'ADC (en kHz)
#define Fe_Khz 10.0

//==========END USER DEFINE========================================================================================

// ========= Variable globales indispensables et déclarations fct d'IT ============================================
#define Te_us (1000.0/Fe_Khz) // Te_us est la période d'échantillonnage en float, exprimée en µs
void IT_Principale(void);
void IT_Ext_3V3(void);
//=================================================================================================================


/*=================================================================================================================
 					FONCTION MAIN : 
					NB : On veillera à allumer les diodes au niveau des E/S utilisée par le progamme. 
					
					EXEMPLE: Ce progamme permet de générer une PWM (Voie 1) à 20kHz dont le rapport cyclique se règle
					par le potentiomètre de "l'entrée Analogique +/-10V"
					Placer le cavalier sur la position "Pot."
					La mise à jour du rapport cyclique se fait à la fréquence 1kHz.

//=================================================================================================================*/



int main (void)
{
// !OBLIGATOIRE! //		
Conf_Generale_IO_Carte();

//______________ Ecrire ici toutes les CONFIGURATIONS des périphériques ________________________________

	
// Paramétrage ADC pour entrée analogique
Conf_ADC();
	
// Conf IT
Conf_IT_Principale_Systick(IT_Principale, Te_us);
	
// Configuration de la PWM avec une porteuse Triangle 
Triangle (FPWM_Khz);
Active_Voie_PWM(1);	
Active_Voie_PWM(2);		
Start_PWM;
R_Cyc_1(2048);  // positionnement à 50% par défaut de la PWM
	R_Cyc_2(2048);
	Inv_Voie(2);

// Activation LED
LED_Courant_Off;
LED_PWM_On;
LED_PWM_Aux_Off;
LED_Entree_10V_On;
LED_Entree_3V3_Off;
LED_Codeur_Off;

	while(1)
	{}

}





//=================================================================================================================
// 					FONCTION D'INTERRUPTION PRINCIPALE SYSTICK
//=================================================================================================================

int Cy_1, Cy_2, Cy_3;
int Courant_1,Courant_2,Courant_3,In_10V;
float erreur ;
float commande_prec = 0 ; 
float commande ;
float a,b;
float Tau_1 = 0.002 ;
float Tau_2 = 0.0016 ;
float erreur_prec = 0 ; // intérogation sur la valeur initiale
float erreur_analogique ;
float commande_num;

void IT_Principale(void)
{
 //commande et retour courant 
 In_10V=Entree_10V();
 Courant_1 = I1();
 
	//calcul de l'erreur
 erreur = In_10V-Courant_1 ; 

	//correcteur 
	a = (Te_us/(1000000))/(2*Tau_2) + (Tau_1)/(Tau_2) ;
	b = (Te_us/(1000000))/(2*Tau_2) - (Tau_1)/(Tau_2) ;
	
  //passage en analogique 
	erreur_analogique = erreur * (3.3/4095);
	
	// Equation de réccurence 
	commande = erreur_analogique*a + erreur_prec*b + commande_prec ;
	
	//Ici on borne notre commande 
	/*if (commande > 1.485){ // 95% 
		commande = 1.485;
	}
	if (commande < -1.485){	// 5% 
		commande = -1.485;
	}*/
	
	//actualisation des valeurs 
	erreur_prec = erreur_analogique ;
	commande_prec = commande ;
	//passage en numérique pour commander la PWM
	commande_num = commande * (4095/3.3);
	
	//PWM centrée sur 50% en ajoutant 2047
  R_Cyc_1((int)commande_num +2047);
	R_Cyc_2((int)commande_num +2047);
}

//=================================================================================================================
// 					FONCTION D'INTERRUPTION EXTERNE (Entrée 0/3V3) 
//=================================================================================================================

void IT_Ext_3V3(void)
{
}
