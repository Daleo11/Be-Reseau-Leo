# BE RESEAU
## BE Reseau - 3 MIC -Dalquier Leo

## Versions
les versions faites vont jusqu'à la version MICTCP-v3 donc la fiabilité partielle.

La variable fiabilite_totale presente dans la version 3(fiabilité partielle) permet de passer de la version 2 à la version 3.
Si elle vaut [1] alors on est sur la version 2 si elle vaut [0] alors on est sur la version 3

L'envois de message format texte fonctionne pour la video elle fonctionne parfois mais pas constament.

## Tags

Le fichier issue de la fiabilité totale n'est pas propre car cela à été fait à la toute fin des differentes versions

1. Fiabilité totale : https://github.com/Daleo11/Be-Reseau-Leo/releases/tag/version-2

2. Fiabilité partielle : https://github.com/Daleo11/Be-Reseau-Leo/releases/tag/version-3


## Details sur le lancement des programmes

En version totale majoritairement, les premieres compilation puis executions on tendance à échouer avec une boucle infinie denvois de message qui n'aboutissement pas dès qu'on ecrit le message.
Il faut plusieur tentative (parfois 5) pour que le message s'envoi correctement, ensuite cela fonctionne.
Ce n'est pas toujours le cas mais j'ai déjà pu le constater plusieurs fois.

## Version fiabilité totale choix d'implémentation

La fiabilité totale doit renvoyer le message jusqu'a ce que ça fonctionne.
Le choix fait est un premier envoi si il a reussi alors on recupere l'ack,
Une boucle while permet d'envoyer jusqu'à succès, la condition de sortie étant qu'il n'y a pas de perte.


On considère qu'il y a une perte si l'envoi echoue, la reception echoue ou que l'ack n'est pas le bon.

Après avoir reussi à envoyer (être sortie de la boucle) on incremente le PE.
Un compteur est présent pour savoir combien de tentative on été necessaires.


Coté recepteur on verifie que le PE soit le bon (si ce n'est pas le bon on affiche "mauvais PE" mais on ne fait rien)
Si c'est le bon PE alors on récupère le payload et on incrémente le PE.
On envoi a chaque fois un message avec la balise ACK activée et seq_num=PE et ack_num=PE (car le PE a ete incrementer si on a eu une reussite)
## Version fiabilité partielle choix d'implémentation

La fiabilité partielle doit ignorer l'echec si il est jugé tolerable c'est à dire que nb_perte>=nb_perte_tolere le nb_perte est donner par la fonction compte_perte().
Si la perte n'est pas tolerable alors on considère qu'on est en fiabilité totale on entre alors dans le while et n'en sort que si il n'y a pas de perte.

La différence majeur est donc qu'il faut ajouter la condition sur le nb de perte sinon le reste du travail est le meme que la version-2.

Contrairement à la version-2 si on depasser le while ce n'est pas forcement qu'on a réussit à envoyer le message on peut aussi avoir ignoré la perte, on verifie donc si il y a une perte ou non.
Si il y en a une alors on est pas rentré dans la boucle (car c'est l'une des conditions de sortie qui doit être validée)
il ne faut donc pas incrementer le PE.
Si il n'y a pas de perte alors on incrémente le PE.

## Commandes de compilation

Pour compiler : make
Pour lancer le code :
texte :

        ./tsock_texte -p port

        ./tsock_texte -s localhost port

video :

./tsock_video -p -t mictcp

./tsock_video -s -t mictcp

Attention pour utiliser la vidéo 2 lignes sont à changer dans la partie send de mictcp l129 et l137 doivent etre décommentées et l130 et l138 doivent être commentées et vise versa
