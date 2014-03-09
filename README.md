MailCPP
=======

C++ implementation of a SMTP client (in French), under Apache 2.0 Licence.

Compatibilité
-------------
Ce programme est écrit en C++ et est compatible avec les distributions Linux. Malheureusement celui-ci est incompatible avec le système Windows qui utilisent un autre système de socket.

Il est cependant possible de l’exécuter sur l’environnement Windows via, par exemple, Cygwin, qui permet la simulation de l’environnement Linux.
Pour fonctionner correctement avec les caractères spéciaux, il est recommandé d’utiliser une invite de commande (terminal) utilisant le jeu de caractère (charset) UTF-8.

Cygwin est téléchargeable à l’adresse : http://www.cygwin.com/

Code source et compilation
--------------------------
Le code source de l’application est présent dans « smtp.cpp » (encodage UTF-8) et peut-être compilé, par exemple, à l’aide de g++ via la commande suivante : 
g++ ./smtp.cpp -o ./smtp

Ce qui créera le programme « smtp » dans le répertoire courant.

Utilisation
-----------
Le programme se nomme « smtp » et s’utilise de la manière suivante :
./smtp   serveur

Le paramètre à indiquer lors d’un appel à ce programme est :
* serveur : Adresse IP ou nom d'hôte du serveur SMTP.

Fichier de log
--------------
Pour ce programme, un fichier de log « smtp.log » est utilisé, celui-ci contient toutes les communications effectuées entre le serveur et le présent programme d’envoi d’email. 

Fonctionnement
--------------
Si vous appelez le programme sans le paramètre obligatoire, celui-ci affichera un texte d’indication pour vous permettre de le lancer correctement, en décrivant la fonction du programme et les paramètres à passer à celui-ci pour son fonctionnement.
 
Une fois le paramètre correctement entré, le programme va fonctionner de la manière suivante : Tout d’abord, celui-ci va créer un fichier log ou l’écraser si celui-ci existe déjà.

Ensuite, le programme va créer un socket et le connecter au port 25 (SMTP) du serveur distant indiqué en paramètre. Si la connexion réussie, celui-ci va initialiser la conversation avec le serveur (commande « HELO »), puis va récupérer auprès de l’utilisateur l’expéditeur et le destinataire de l’email et les transmettre au serveur (respectivement avec les commandes « MAIL FROM : » et « RCPT TO : »). Si un de ceux-ci est refusé, le programme boucle pour demander à nouveau à l’utilisateur l’expéditeur ou le destinataire et le retransmet.

Si le serveur répond à la transmission du destinataire avec un code d’erreur 4xx, c’est-à-dire qu’il s’agit d’un greylisting (rejet temporaire), le programme bouclera toutes les minutes pour tenter à nouveau de transmettre le destinataire. Ce message de rejet temporaire ne peut apparaître que comme réponse lors de l’envoi du destinataire du message, puisque le greylisting est basé sur triplet identifié par l’adresse IP de l’émetteur, l’adresse mail de l’expéditeur et l’adresse mail du destinataire, triplet qu’il ne possède qu’en recevant ce dernier.

Si aucune erreur ne s’est manifestée, le programme va engager la partie des données (via la commande « DATA »), demander à l’utilisateur le sujet de l’email et le transmettre au serveur accompagnée des informations complémentaires tel que l’expéditeur et le destinataire (ceci pour que les clients emails puissent interpréter ceux-ci).

Finalement, l’utilisateur est invité à indiquer le contenu de son message, qui peut comprendre plusieurs lignes. Celui-ci prend fin une fois que l’utilisateur entre une ligne ne contenant qu’un seul point. Dès lors le programme détecte cette fin et transmet le point accompagnée de la commande de fermeture de la conversation avec le serveur (commande « QUIT »).

Pour chacune des étapes précédemment citées, le programme vérifiera la réussite de chaque commande via le code envoyé en réponse par le serveur (un code de type 5xx indiquant une erreur) et stoppera le script en cas d’erreur (à l’exception des erreurs d’adresse expéditeur ou destinataire). 