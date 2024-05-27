#include <mictcp.h>
#include <api/mictcp_core.h>

//Variables
unsigned short no_port_dest;
unsigned short no_port_local;
mic_tcp_sock_addr * addresse_sock;
mic_tcp_sock socket1;
int PE=0;
int nb_perte_autorise=2;//pourcentage de perte autoriser
int taille_fenettre=10;//si taille_fenettre > taille reelle de la fenettre bug sinon ce nest pas grave simplement une partie de la fenettre sera inutiliser si la variable est inferieur a la talle reelle
int fenettre[10];//idealement doit etre = a taille_fenettre (si > pas grave si < bug)
int fiabilite_totale=0;//0 si perte, 1 si aucune perte tolere
//le ratio de perte autoriser est nb_perte_autorise/taille_fenettre (actuellement 20%)


//fction pr manipuler la fenettre glissante
void init_fenetre(){
    for (int i=0;i<taille_fenettre;i++){
        fenettre[i]=0;//0 pas de perte, 1 perte
    }
}

int compte_perte(){//renvoi nb perte enregistrées dans la fenettre
    int res=0;
    for (int i=0;i<taille_fenettre;i++){
        res=res+fenettre[i];
    }
    return res;
}

void decalage_fenetre(){//fait "glisser" la fenettre
    for (int i=1;i<taille_fenettre;i++){
        fenettre[i-1]=fenettre[i];
    }
}

void ajout_fenettre(int val){//ajoute l'element a la fin de la fenettre
    decalage_fenetre();
    fenettre[taille_fenettre-1]=val;
}

void aff_fenettre(){//affiche les valeurs contenues dans la fenettre
    for (int i=0;i<taille_fenettre;i++){
        printf("%d ",fenettre[i]);
    }
    printf("\n");
}



/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(50);//pourcentage perte
   if (result!=-1){//si pas echec
        socket1.fd=0;//on met l'identificateur a la val 0
        socket1.state=IDLE;
        return socket1.fd;// on renvoi l'indentificateur du socket
   }

   return result; //si on arrive ici result=-1 
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)//addr locale
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   memcpy((char*)&socket1.local_addr,(char*)&addr,sizeof(mic_tcp_sock_addr));
   return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{  
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)//addr distante
{   
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    socket1.remote_addr=addr;
    printf("INIT Fenettre\n");
    init_fenetre();
    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_pdu pdu;
    pdu.header.source_port=socket1.local_addr.port;
    pdu.header.dest_port=socket1.remote_addr.port;
    pdu.header.seq_num=PE;
    pdu.header.ack_num=(PE+1)%2;
    pdu.header.syn=0;
    pdu.header.ack=0;
    pdu.header.fin=0;
    pdu.payload.data=mesg;
    pdu.payload.size=mesg_size;
    
    int res1=IP_send(pdu,socket1.remote_addr.ip_addr);//tentative d'envoi numero 1
    //passage en reception pour recuperer ACK
    int res2=0;
    int tentative=1;
    mic_tcp_pdu pdu2;
    if (res1!=0){// ca sert a rien dessayer de receptionner un ACK si lenvoi a echouer donc on verifie
        //res2=IP_recv(&pdu2,&socket1.local_addr.ip_addr,NULL,50);//si video
        res2=IP_recv(&pdu2,&socket1.local_addr.ip_addr,&socket1.remote_addr.ip_addr,1000);//si texte
    }

    int perte=pdu2.header.ack_num==PE || res1==-1 || res2==-1;
    printf("test %d %d\n",compte_perte()<nb_perte_autorise,perte);
    printf("%d %d %d %d %d\n",pdu2.header.ack_num,PE,res1,res2,(compte_perte()<nb_perte_autorise));
    //         perte   et   perte non tolerable                       perte et totale
    while ((perte && (compte_perte()<nb_perte_autorise)==0) || (perte && fiabilite_totale)){//si ACK correspond pas ou echec envoi ou timeout
        res1=IP_send(pdu,socket1.remote_addr.ip_addr);
        //res2=IP_recv(&pdu2,&socket1.local_addr.ip_addr,NULL,50);//si video
        res2=IP_recv(&pdu2,&socket1.local_addr.ip_addr,&socket1.remote_addr.ip_addr,1000);//si texte
        
        tentative++;
        
        perte=pdu2.header.ack_num==PE || res1==-1 || res2==-1;
        //printf("%d %d %d %d %d\n",pdu2.header.ack_num,PE,res1,res2,(compte_perte()<nb_perte_autorise));//differentes infos utiliser pour le debugage

    }
    
    if (perte){//si perte
        ajout_fenettre(1);//on rajoute une perte
    }
    else{
        ajout_fenettre(0);//on rajoute un succes
        PE=(PE+1)%2;//si vaut 0 devient 1 et vise versa
    }

    //printf("nb tentative : %d\n",tentative);//nombre de tentative pour un envoi (si=1 alors ca a marché du premier coup)
    //aff_fenettre();//permet de voir l'etat de la fenettre
    //printf("nb perte dans fenettre %d\n",compte_perte());//renvoi le nombre de perte qui sont presente sur la fenettre actuelle
    int res=res1;
    
    return res;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_pdu pdu;
    pdu.payload.data=mesg;
    pdu.payload.size=max_mesg_size;
    int res=app_buffer_get(pdu.payload);//la verification du PE est faite avant si y a rien cest soit quon a rien recu soit que le PE correspond pas
    

    return res;
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_ip_addr local_addr, mic_tcp_ip_addr remote_addr)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    //verification PE
    if (pdu.header.seq_num==(PE)){
        app_buffer_put(pdu.payload);//on recup le message qui est dans les buffers
        printf("PE OK\n");
        PE=(PE+1)%2;
    }
    else{
        printf("mauvais PE\n");//se produit qd il y a une perte de l'ack
    }
    mic_tcp_pdu pdu2;
    pdu2.header.source_port=socket1.local_addr.port;
    pdu2.header.dest_port=socket1.remote_addr.port;
    pdu2.header.seq_num=(PE+1)%2;
    pdu2.header.ack_num=PE;
    pdu2.header.syn=0;
    pdu2.header.ack=1;
    pdu2.header.fin=0;
    pdu2.payload.size=0;
    //printf("envoi ACK %d %d\n",pdu2.header.ack_num,pdu2.header.seq_num);//info pr debuggage
    IP_send(pdu2,remote_addr);

} 

