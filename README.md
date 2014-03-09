MailCPP
=======

C++ implementation of a SMTP client (in French), under Apache 2.0 Licence.

Compatibilit�
-------------
Ce programme est �crit en C++ et est compatible avec les distributions Linux. Malheureusement celui-ci est incompatible avec le syst�me Windows qui utilisent un autre syst�me de socket.

Il est cependant possible de l�ex�cuter sur l�environnement Windows via, par exemple, Cygwin, qui permet la simulation de l�environnement Linux.
Pour fonctionner correctement avec les caract�res sp�ciaux, il est recommand� d�utiliser une invite de commande (terminal) utilisant le jeu de caract�re (charset) UTF-8.

Cygwin est t�l�chargeable � l�adresse : http://www.cygwin.com/

Code source et compilation
--------------------------
Le code source de l�application est pr�sent dans � smtp.cpp � (encodage UTF-8) et peut-�tre compil�, par exemple, � l�aide de g++ via la commande suivante : 
g++ ./smtp.cpp -o ./smtp

Ce qui cr�era le programme � smtp � dans le r�pertoire courant.

Utilisation
-----------
Le programme se nomme � smtp � et s�utilise de la mani�re suivante :
./smtp   serveur

Le param�tre � indiquer lors d�un appel � ce programme est :
* serveur : Adresse IP ou nom d'h�te du serveur SMTP.

Fichier de log
--------------
Pour ce programme, un fichier de log � smtp.log � est utilis�, celui-ci contient toutes les communications effectu�es entre le serveur et le pr�sent programme d�envoi d�email. 

Fonctionnement
--------------
Si vous appelez le programme sans le param�tre obligatoire, celui-ci affichera un texte d�indication pour vous permettre de le lancer correctement, en d�crivant la fonction du programme et les param�tres � passer � celui-ci pour son fonctionnement.
 
Une fois le param�tre correctement entr�, le programme va fonctionner de la mani�re suivante : Tout d�abord, celui-ci va cr�er un fichier log ou l��craser si celui-ci existe d�j�.

Ensuite, le programme va cr�er un socket et le connecter au port 25 (SMTP) du serveur distant indiqu� en param�tre. Si la connexion r�ussie, celui-ci va initialiser la conversation avec le serveur (commande � HELO �), puis va r�cup�rer aupr�s de l�utilisateur l�exp�diteur et le destinataire de l�email et les transmettre au serveur (respectivement avec les commandes � MAIL FROM : � et � RCPT TO : �). Si un de ceux-ci est refus�, le programme boucle pour demander � nouveau � l�utilisateur l�exp�diteur ou le destinataire et le retransmet.

Si le serveur r�pond � la transmission du destinataire avec un code d�erreur 4xx, c�est-�-dire qu�il s�agit d�un greylisting (rejet temporaire), le programme bouclera toutes les minutes pour tenter � nouveau de transmettre le destinataire. Ce message de rejet temporaire ne peut appara�tre que comme r�ponse lors de l�envoi du destinataire du message, puisque le greylisting est bas� sur triplet identifi� par l�adresse IP de l��metteur, l�adresse mail de l�exp�diteur et l�adresse mail du destinataire, triplet qu�il ne poss�de qu�en recevant ce dernier.

Si aucune erreur ne s�est manifest�e, le programme va engager la partie des donn�es (via la commande � DATA �), demander � l�utilisateur le sujet de l�email et le transmettre au serveur accompagn�e des informations compl�mentaires tel que l�exp�diteur et le destinataire (ceci pour que les clients emails puissent interpr�ter ceux-ci).

Finalement, l�utilisateur est invit� � indiquer le contenu de son message, qui peut comprendre plusieurs lignes. Celui-ci prend fin une fois que l�utilisateur entre une ligne ne contenant qu�un seul point. D�s lors le programme d�tecte cette fin et transmet le point accompagn�e de la commande de fermeture de la conversation avec le serveur (commande � QUIT �).

Pour chacune des �tapes pr�c�demment cit�es, le programme v�rifiera la r�ussite de chaque commande via le code envoy� en r�ponse par le serveur (un code de type 5xx indiquant une erreur) et stoppera le script en cas d�erreur (� l�exception des erreurs d�adresse exp�diteur ou destinataire). 