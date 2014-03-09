/**
 * Système d'envoi d'un mail via un serveur SMTP
 * =============================================
 * @version 1.0
 * @date 11.06.2012
 * @author Chavaillaz Johan
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdio.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fstream>
#include <cstring> 
#include <cstdlib>

using namespace std;

#define LIGNE "\r\n"
#define TAILLE_CMD 300
#define COULEUR_ROUGE "\33[31m"
#define COULEUR_VERTE "\33[32m"
#define COULEUR_JAUNE "\33[33m"
#define COULEUR_BLEU "\33[01;34m"
#define GRAS "\33[1m"
#define SOULIGNE "\33[4m"
#define NORMAL "\33[0m"

int sock;
struct sockaddr_in serveur;
struct hostent *hp, *gethostbyname();
char buffer[BUFSIZ+1];
int tailleRetourSocket;
ofstream LOG;

/**
 * Fonction permettant d'extraire une partie de la chaine
 */
char* substring(char* chaine, size_t debut, size_t longueur) 
{
	// Vérification des paramètres
	if (chaine == 0 || strlen(chaine) == 0 || strlen(chaine) < debut || strlen(chaine) < (debut+longueur)) 
		return 0; 
	
	// Extraction de la chaine désirée
	return strndup(chaine + debut, longueur); 
} 

/**
 * Fonction permettant de vider le buffer
 */
void viderBuffer()
{
    int c = 0;
	
	// Boucle pour récupérer la fin de la chaine 
	// Jusqu'au caractère de retour à la ligne ou fin de ligne
    while (c != '\n' && c != EOF)
    {
		// Lecture caractère par caractère du buffer
        c = getchar();
    }
}

/**
 * Fonction permettant la lecture d'une chaine au clavier
 * Avec sécurisation de la saisie et du buffer
 */
int lireClavier(char *chaine, int longueur, bool supprimerRetour = true)
{
    char *positionEntree = NULL;
	
	// Lecture du texte saisi au clavier s'il n'y a pas d'erreur
    if (fgets(chaine, longueur, stdin) != NULL)
    {
		// Recherche le caractère de retour à la ligne
        positionEntree = strchr(chaine, '\n');
		
		// Si le caractère existe
        if (positionEntree != NULL && supprimerRetour == true)
        {
			// Remplace le caractère par \0
            *positionEntree = '\0';
        }
        else
        {
			// Vide le buffer
            viderBuffer();
        }
		
		// Indique que la chaine a été récupérée
        return 1;
    }
    else
    {
		// Vide le buffer
        viderBuffer();
		
		// Indique qu'aucune chaine n'a été récupérée
        return 0;
    }
}

/**
 * Fonction permettant d'envoyer une chaine de caractère à un socket
 */
void ecrireSocket(char* commande)
{
	// Envoi au socket
	write(sock, commande, strlen(commande));
	
	// Débogage
	//write(1, commande, strlen(commande));
	//printf(">> Client: %s \r\n", commande);
	
	// Enregistrement de la commande
	LOG << commande;
}

/**
 * Fonction permettant de lire une chaine reçu depuis un socket
 */
char lireSocket()
{
	// Lecture depuis le socket
	tailleRetourSocket = read(sock, buffer, BUFSIZ);
	
	// Débogage
	//write(1, buffer, tailleRetourSocket);
	//printf(">> Serveur: %s \r\n", buffer);
	
	// Enregistrement de la réponse
	LOG << substring(buffer, 0, tailleRetourSocket);
	
	return buffer[0];
}

/**
 * Fonction permettant de récupérer l'information fournie après le code lors d'un message
 */
char* informationSocket()
{
	// Retourne l'information envoyée par le serveur (juste après le code à 3 chiffre)
	return substring(buffer, (size_t) 3, (size_t) (tailleRetourSocket - 4));
}

/**
 * Fonction permettant de compiler une commande pour l'envoyer au serveur
 */
char* compilerCommande(char* commande, char* donnees = (char*)"")
{
	// Création de la chaine pour la commande
	char* compilation = new char[TAILLE_CMD];
	
	// Concatène les instructions de la commande
	strcpy(compilation, commande);
	strcat(compilation, donnees);
	strcat(compilation, LIGNE);
	
	// Retourne la nouvelle chaine
	return compilation;
}

/**
 * Entrée principale du programme
 */
int main(int argc, char* argv[])
{
	char expediteur[250], destinataire[250], sujet[250], corps[250];
	char commande[TAILLE_CMD];
	char statut;
	
	// Vérification de la présence du paramètre
	if (argc != 2)
	{
		// S'il n'est pas présent, affichage d'un message d'aide
		cout << endl << "  SMTP Mail" << endl;
		cout << "  =========" << endl;
		cout << "  Permet d'envoyer un mail simple en utilisant le serveur SMTP choisi." << endl << endl;
		cout << "  Syntaxe :  ./smtp   " << COULEUR_BLEU << "serveur" << endl;
		cout << "    - serveur " << NORMAL << ": Adresse IP ou nom d'hôte du serveur SMTP" << endl << endl;
		cout << "  Par Johan Chavaillaz" << endl;
		
		// Fermeture du programme
		return EXIT_SUCCESS;
	}
	
	// Ouverture en écriture avec effacement du fichier log
	LOG.open("smtp.log", ios::out | ios::trunc);
	
	// En cas d'impossibilité d'ouvrir / créer le fichier
	if (!LOG) 
	{ 
		// Indication de l'erreur dans le flux approprié
		cerr << COULEUR_ROUGE << "[ERREUR] Impossible d'ouvrir en écriture ou de créer le fichier log." << NORMAL << endl;
		
		// Fermeture du programme avec indication d'erreur
		return EXIT_FAILURE;
	}
	
	// Création d'un socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	// En cas d'erreur de la créationd du socket
	if (sock == -1) 
	{
		// Indication de l'erreur dans le flux approprié
		cerr << COULEUR_ROUGE << "[ERREUR] Création d'un socket impossible." << NORMAL << endl;
		
		// Fermture du fichier log
		LOG.close();
		
		// Fermeture du programme avec indication d'erreur
		return EXIT_FAILURE;
	}
	
	try 
	{
		// Si on arrive ici, le socket a été créé
		cout << COULEUR_VERTE << "[INFO] Socket créé avec succès." << NORMAL << endl;
		
		// Définition de la famille de socket à utiliser
		serveur.sin_family = AF_INET;
		
		// Récupération d'informations sur le serveur
		hp = gethostbyname(argv[1]);
		
		// Vérification de l'hôte indiqué
		if (hp == (struct hostent *) 0) throw "Serveur inconnu.";

		// Copie l'adresse récupérée depuis getHostByName dans la configuration du socket
		memcpy((char *) &serveur.sin_addr, (char *) hp->h_addr, hp->h_length);
		
		// Configure pour utiliser le port 25 (Port SMTP)
		serveur.sin_port = htons(25);
		
		// Création d'une connexion à l'hôte avec la configuration
		if (connect(sock, (struct sockaddr *) &serveur, sizeof serveur) == -1)
		{
			// Affichage d'un message d'erreur
			throw "Connexion du socket au serveur SMTP échouée";
		}
		
		// Si on arrive ici, la connexion a été effectuée
		cout << COULEUR_VERTE << "[INFO] Connexion au serveur SMTP effectuée." << NORMAL << endl;
		
		// Lecture de la chaine de connexion affichée par le serveur
		lireSocket(); 
		
		// Initialisation de conversation avec le serveur
		strcpy(commande, compilerCommande((char*)"HELO ", argv[1]));
		ecrireSocket(commande);
		statut = lireSocket();
		
		// Vérification de la réussite de cette commande
		if (statut != '2') throw "Commande d'initialisation avec le serveur echouée.";
		
		do
		{
			// Si une boucle s'est déjà produite, indication de l'erreur
			if (statut != '2') cerr << COULEUR_ROUGE << "[ERREUR]" << informationSocket() << NORMAL;
			
			// Récupération de l'expéditeur auprès de l'utilisateur
			cout << endl << GRAS << "Veuillez indiquer le mail de l'expéditeur : " << NORMAL;
			lireClavier(expediteur, 250);
			
			// Spécifie l'expéditeur du message au serveur
			strcpy(commande, compilerCommande((char*)"MAIL FROM: ", expediteur));
			ecrireSocket(commande);
			statut = lireSocket();
		}
		while(statut != '2');
		
		// Information de réussite
		cout << COULEUR_VERTE << "[INFO] Expéditeur validé par le serveur. " << NORMAL << endl;
		
		do
		{
			// Si une boucle s'est déjà produite, indication de l'erreur
			if (statut != '2') cerr << COULEUR_ROUGE << "[ERREUR]" << informationSocket() << NORMAL;
			
			// Récupération du destinataire auprès de l'utilisateur
			cout << endl << GRAS << "Veuillez indiquer le mail du destinataire : " << NORMAL;
			lireClavier(destinataire, 250);
			
			// Spécifie le destinataire du message au serveur
			strcpy(commande, compilerCommande((char*)"RCPT TO: ", destinataire));
			ecrireSocket(commande);
			statut = lireSocket();
			
			// Si il y a greylisting, on demande en boucle (avec attente)
			while (statut == '4')
			{
				// Avertissement de l'utilisateur
				cout << COULEUR_JAUNE << "[AVERTISSEMENT] Greylisting, nouvelle tentative dans 1 minute" << NORMAL << endl;
				
				// Attente d'une minute
				sleep(60);
				
				// Nouvelle demande
				ecrireSocket(commande);
				statut = lireSocket();
			}
		}
		while(statut != '2');
		
		// Information de réussite
		cout << COULEUR_VERTE << "[INFO] Destinataire validé par le serveur. " << NORMAL << endl;
		
		// Commencement des données du mail
		strcpy(commande, compilerCommande((char*)"DATA"));
		ecrireSocket(commande);
		statut = lireSocket();
		
		// Vérification qu'il s'agit d'un message de validation pour le contenu du mail
		if (statut != '3') throw informationSocket();
		
		// Récupération du sujet du mail auprès de l'utilisateur
		cout << endl << GRAS << "Veuillez indiquer le sujet du mail : " << NORMAL << endl;
		lireClavier(sujet, 250);
		
		// Spécifie le sujet du mail
		strcpy(commande, compilerCommande((char*)"Subject: ", sujet));
		ecrireSocket(commande);
		
		// Spécifie dans le contenu du mail l'expéditeur
		strcpy(commande, compilerCommande((char*)"From: ", expediteur));
		ecrireSocket(commande);
		
		// Spécifie dans le contenu du mail le destinataire
		strcpy(commande, compilerCommande((char*)"To: ", destinataire));
		ecrireSocket(commande);
		
		// Récupération du corps du message auprès de l'utilisateur
		cout << endl << GRAS << "Veuillez indiquer votre message ";
		cout << "(terminez par une ligne avec un point seul) : " << NORMAL << endl;
		
		// Met une ligne vide
		strcpy(commande, compilerCommande((char*)""));
		ecrireSocket(commande);
		
		// Boucle pour le corps du message
		do
		{
			// Lecture de chaque ligne entrée
			lireClavier(corps, 250);
			
			// S'il s'agit de la fin du message
			if (strcmp(".", corps) == 0)
			{
				// Met une ligne vide
				strcpy(commande, compilerCommande((char*)""));
				ecrireSocket(commande);
			}
			
			// Transmission de la ligne au serveur mail
			strcpy(commande, compilerCommande(corps));
			ecrireSocket(commande);
		}
		// Tant que la saisie n'est pas un point
		while (strcmp(".", corps) != 0);
		
		// Fermeture de la conversation avec le serveur
		strcpy(commande, compilerCommande((char*)"quit"));
		ecrireSocket(commande);
		statut = lireSocket();
		
		// Vérification, the last one !
		if (statut == '2')
		{
			// Message de réussite
			cout << COULEUR_VERTE << "[INFO] Message envoyé avec succès avec le serveur SMTP." << NORMAL << endl;
		}
		else
		{
			// Message d'erreur
			throw informationSocket();
		}
	}
	// Récupération des erreurs critiques
	catch(const char* &exception)
	{
		cerr << COULEUR_ROUGE << "[ERREUR] " << exception << NORMAL << endl;
	}
	
	// Fermeture du fichier log
	LOG.close();
	
	// Fermeture du socket
	close(sock);
	
	// Fermeture du programme
	return EXIT_SUCCESS;
}
