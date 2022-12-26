# Projet-Objets-connect-s
Projet MQTT dans le cadre du  cours d'objet connectés MA1 ISIB

Ce projet permet la communication via MQTT entre différents objets dans l'optique d un jeu. 
C'est un jeu  de rapidité où il faudra scanner un tag RFID le plus rapidement possible sur un des boitier disposé dans une grande pièce. En effet, à chaque mur il y a un boitier qui lorsqu'il  s'illumine indique qu'il faudra s'y diriger et scanner le tag de son équipe.

En parralèle, sur le boitier principal, on mène un compte des scores qui seront affichés en temps réel sur un serveur web créé pour l'occasion. 

Ce site possède également une partie "/commande" qui permet au bar man de mener le compte des commandes. Puisque sur chaque table se trouve également un boitier avec des boutons qui permettent de passer commande des différents cocktails eux aussi interfacés via MQTT.

Pour ce faire nous utilisons 2 topics : Game1 et Commande. Sur ces topics nous communiquons 1 lettre (C,T,M,W) qui indique le type de message et un chiffre qui indique le boitier.

C - commande
T - Table
M - Mur
W - Winer

Par exemple si nous envoyons M1 sur le topic Game1 --> cela allume le boitier mural 1 --> dès qu'un joueur y scanner son tag le boitier communique W + numéro  de l'équipe gagante. 

Idem pour les commandes.