#include "stm32f10x.h"

//=======================================================================================
// 	Auteur T.R.
//  Juin 2016 , 28/06
//  Processeur : STM32F103RB
//  Logiciel dev : �Vision 4.70
// 
// Modif 28/06 ajout fonctionnalit� IT en transmission str :
// void Init_IT_Serial_Transmit_Str(USART_TypeDef *USART, void (*IT_function) (void))
//=======================================================================================


#ifndef _USART_H__
#define _USART_H__



/* ======================================================================================
**************** EMISSION : PRINCIPE fct Put_String *****************************************
Chaine � transmettre = azerty.
En dur : 0x61 0x7A 0x65 0x72 0x74 0x79 0x0 ; Le 0 traduit la fin de la cha�ne. C'est universel.

La fonction Put_String se contente d'�mettre  0x61 0x7A 0x65 0x72 0x74 0x79
Elle aura d�tect� le \0 et ne l'aura pas �mis. Ou si \0 n'existe pas, alors
la transmission s'arr�tera lors d'un d�passement max (voir define)
AUCUN CARACTERE DE CONTROLE N EST EMIS EN PLUS.

Pour �tre dans les clous et envoyer selon l'OS un ou plusieurs caract�res de contr�le:
0x61 0x7A 0x65 0x72 0x74 0x79 \n \r  (\n = LF = 0xA et \r = CR = 0xD)
ce sera � la couche service de dessus d'envoyer deux cha�nes :
0x61 0x7A 0x65 0x72 0x74 0x79 0x0
Puis
0x0A 0x0D 0x0


**************** RECEPTION *****************************************
Tous les protocoles finissent par CR mais sait-on jamais... MAC OS en \n ?
La d�tection du dernier caract�re sera donc au choix.
Exemple :
0x61 0x7A 0x65 0x72 0x74 0x79 \n \r  donnera 
0x61 0x7A 0x65 0x72 0x74 0x79 \n \0 en dur
Eventuellement en couche service on pourra ou non supprimer \n si le protocole place
en fin de cha�ne \n \r

OPTION EN RECEPTION:
le mode ECHO permet de renvoyer tous les caract�res trait�s.

======================================================================================
*/




//===============================ressources sur le STM32F103RB===========================
// Rappel sur les ressources sur le STM32F103RB 
//
// 3 USART possibles :
// USART_1_TX = TIM1_CH2 = PA9
// USART_1_RX = TIM1_CH3 = PA10
// USART_2_TX = TIM2_CH3 = PA2
// USART_2_RX = TIM2_CH4 = PA3
// USART_3_TX = PB10
// USART_3_RX = PB11
//=======================================================================================


/*  TO DO...

Faire les 2 autres usart
Faire un bilan des tests.
Mettre en place la DMA �mission.
Ajouter une fonction Etat Error USART (over run, bruit.)
*/

//===================================INFOS GENRALES======================================
/*
Voir fin de fichier pour le d�tail des fonctions


Le module permet d'envoyer et recevoir octet / string sur USART 1 � 3

Tout est g�r� par IT USART enfouie, priorit� � donner.
Aucun test bloquant avant d'�mettre est pr�vu (sauf flag mat�riel qui temporisent juste � l'appel
d'un Put_Char ou Put_String). Au user de faire attention.
De nombreux flags permettent de savoir si il y a une op�ration transmit / receive en cours

Voici typiquement comment on devrait �crire un put string :
	while(Flag_Transmission_Busy(USART1)==1);	
	Put_String(USART1,"Commande recue");
Ce qui garantit que la COM est libre pour envoyer.
Si le polling n'est pas fait, c'est le bourrage garantit, notamment en cas de full duplex.

Possibilit� de mettre en place un callback � la r�ception d'un byte ou d'un string, sur les 3 USART.

ATTENTION :
Si les callback sont utilis�s, il est primoridal de ne pas y rester pour ne pas bloquer l'IT USART enfouie.
A plus forte raison, il est INTERDIT dans un callback d'appeler une fct em/rec USART car l'IT USART s'interrompt => plantage.
CONSEIL => utiliser cette fonctionnalit� que pour r�cup�rer un flag ou une donn�e � la vol�e,y rester le moins possible.
Mieux => ne pas utiliser les callback mais utiliser un polling r�gulier sur l'appli globale, genre OS.


*/
//=======================================================================================

//_________ USER DEFINE _________________________________________________________________

// Taille du buffer d'�mission (r�glable pour chaque USART)
#define Max_Car_In_String_Emission_1 500
#define Max_Car_In_String_Emission_2 500
#define Max_Car_In_String_Emission_3 500
// Taille du buffer de r�ception (r�glable pour chaque USART)
#define Buf_Len_StrRec_USART1 30
#define Buf_Len_StrRec_USART2 30
#define Buf_Len_StrRec_USART3 30

// Caract�re de terminaison, par d�faut \r = 0xD :
#define Term_Car_USART1 0x0D
#define Term_Car_USART2 0x0D
#define Term_Car_USART3 0x0D

// Mode ECHO lors de la r�ception
#define ECHO_USART1
#define ECHO_USART2
#define ECHO_USART3


#define Null 0
#define ClearScreen 0xC
#define LF 0xA
#define CR 0xD
//=======================================================================================
//                      Configuration  
//=======================================================================================

void Init_Serial_Single_Mode(USART_TypeDef *USART,int Baud_Rate_bits_par_Sec, char Prio_USART); // OK
/*___________________________________________________________________________________________

Permet de pr�parer n'importe laquelle des 3 USART avec un d�bit donn� (8bits, 1 stop, pas de parit�)
Utilise une IT enfouie qui permet la transmission en IT et la r�ception en IT.
___________________________________________________________________________________________*/


void Init_IT_Serial_Receive_Byte(USART_TypeDef *USART, void (*IT_function) (void));
void Init_IT_Serial_Receive_Str(USART_TypeDef *USART, void (*IT_function) (void));
void Init_IT_Serial_Transmit_Str(USART_TypeDef *USART, void (*IT_function) (void));
/*___________________________________________________________________________________________

Ces trois fonctions permettent de renvoyer au user une IT pour indiquer l'�v�nement de r�ception 
Byte ou String.
___________________________________________________________________________________________*/

void Reset_Var_USART(USART_TypeDef *USART); // OK
/*___________________________________________________________________________________________

Ne devrait pas �tre utilis�e... Elle permet de remettre � 0 toutes les variables 
priv�es utilis�e pour l'USART indiqu�e
___________________________________________________________________________________________*/



//=======================================================================================
// 							  flag 
//=======================================================================================
char Flag_Transmission_Busy(USART_TypeDef *USART); // OK
char Flag_Overrun_Transmit(USART_TypeDef *USART);
void Clear_Overrun_Transmit(USART_TypeDef *USART);
/*___________________________________________________________________________________________

Gestion du flag Flag_Transmission_Busy :
G�r� automatiquement :
- mis � un automatiquement � l'appel des deux fonctions put
- mis � 0 automatiquement lors de la fin des processus put

Gestion du flag Flag_Overrun_Transmit :
- mis � un automatiquement � l'appel des deux fonctions put, si une transmission est en cours
- mis � 0 par la fonction RESET_VAR_USART, plus pertinent, pas la fonction VOID CLEAR_OVERRUN_TRANSMIT
___________________________________________________________________________________________*/

char Flag_Str_Received(USART_TypeDef *USART); // OK
char Flag_Byte_Received(USART_TypeDef *USART); //OK
char Flag_Overrun_Receive_Str(USART_TypeDef *USART);
void Clear_Overrun_Receive_Str(USART_TypeDef *USART);

/*___________________________________________________________________________________________

Gestion des Flag de r�ception :
Ils sont remis � 0 soit au reset g�n�ral, soit lors du get
le flag_Byte_Received passe � 1 � chaque r�ception de byte.
Le flag_Str_Received passe � 1 en r�ception dernier caract�re.


Le flag FLAG_OVERRUN_RECEIVE_STR
Mis � 1 en fin de r�ception string si le flag Str received �tait encore � 1.
Remis � 0 au reset g�n�ral et par la fonction CLEAR_OVERRUN_RECEIVE_STR

___________________________________________________________________________________________*/


//=======================================================================================
// 							  Utilisation, rem : // OK signifie valid� en reel serial 1
//=======================================================================================

char Get_Char(USART_TypeDef *USART); //OK
char * Get_String(USART_TypeDef *USART); // OK
int Get_Len_Receiv_String(USART_TypeDef *USART); //OK


void Put_Char(USART_TypeDef *USART,char Car); // OK
void Put_String(USART_TypeDef *USART,char * pCar); // OK
int Get_Len_Transm_String(USART_TypeDef *USART); //OK

/*
Les fonctions d'�mission
=======================================================================================
La fonction PUT_STRING s'arr�te sous deux conditions et ajoute un caract�re de fin (voir define .h):
- rencontre du caract�re null
ou
- atteinte du nombre max de caract�res �mis (voir define).
Elle utilise l'IT USART enfouie
La fonction  Put_Char n'utilise aucune IT. L'�mission est directe.


INT GET_LEN_TRANSM_STRING(USART_TYPEDEF *USART);
Elle permet de r�cup�rer si besoin la longueur du caract�re �mis (caract�re null non compris).
!ATTENTION ! Cette longueur est mise � jour lorsque la transmission est finie. Il faut donc
ins�rer une attente fin de transmission :
	while(Flag_Transmission_Busy(USART1)==1);
	LEM=Get_Len_Transm_String(USART1);	

Les fonctions de r�ception
=======================================================================================
Deux fonctions :
CHAR GET_CHAR(USART_TYPEDEF *USART);
CHAR * GET_STRING(USART_TYPEDEF *USART);

Les deux exploitent l'interruption enfouie USART, en concurrence avec la transmission.
Les drapeaux FLAG_BYTE_RECEIVED et FLAG_STR_RECEIVED passent � 1 pour indiquer que les donn�es
sont pr�tes � �tre lues.
Si les donn�es sont �cras�es le flag FLAG_OVERRUN_RECEIVE passe � 1.

*/





#endif
