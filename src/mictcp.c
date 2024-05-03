#include <mictcp.h>
#include <api/mictcp_core.h>

//Variables
unsigned short no_port_dest;
unsigned short no_port_local;
mic_tcp_sock_addr * addresse_sock;
mic_tcp_sock socket1;
int *PE=0;





/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(0);//pourcentage perte
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
   //socket1.fd=socket;
   //socket1.local_addr.ip_addr.addr=gethostbyname("localhost");
   //memcpy((char*)&socket1.local_addr.ip_addr.addr,(char*)gethostbyname("localhost"),sizeof(char));
   //socket1.local_addr.ip_addr.addr_size=sizeof(addr.ip_addr.addr);
   //socket1.local_addr.port=9000;
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
    //addresse_sock=&addr;
    //no_port_local=addr.port;

    //mic_tcp_pdu pdu;
    //pdu.header.source_port=no_port_local;
    //pdu.header.dest_port=no_port_dest;

    //IP_send(pdu,addr.ip_addr);
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    socket1.remote_addr=addr;
    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_pdu *pdu;
    pdu=malloc(sizeof(mic_tcp_pdu));
    pdu->header.source_port=socket1.local_addr.port;
    pdu->header.seq_num=*PE;
    pdu->header.dest_port=socket1.remote_addr.port;
    pdu->header.ack_num=*PE;
    pdu->header.syn=0;
    pdu->header.ack=0;
    pdu->header.fin=0;
    //pdu->payload.data=malloc(mesg_size);
    pdu->payload.data=mesg;
    pdu->payload.size=mesg_size;
    int res1=IP_send(*pdu,socket1.remote_addr.ip_addr);//tentative d'envoi numero 1
    //passage en reception pour recuperer ACK
    int res2=0;
    mic_tcp_pdu pdu2;
    if (res1==0){// ca sert a rien dessayer de receptionner un ACK si lenvoi a echouer donc on verifie
        res2=IP_recv(&pdu2,&socket1.local_addr,&socket1.remote_addr,1000);
    }
    while (pdu2.header.ack_num!=*PE || res1==-1 || res2==-1){//si ACK correspond pas ou echec envoi ou timeout
        res1=IP_send(*pdu,socket1.remote_addr.ip_addr);
        res2=IP_recv(&pdu2,&socket1.local_addr.ip_addr,&socket1.remote_addr.ip_addr,1000);
        printf("perte message quelque par entre ici et là bas");
        //pour la fiabilité partielle il suffira de mettre manuellement les variables a la valeur necessaire pour sortir de la boucle (res=0, acknum=*PE)
    }

    //fiabilite totale (perte pas acceptable) ->utilisation while ACK pas le bon

    

    int res=res1;
    *PE=1-*PE;//si vaut 0 devient 1 et vise versa
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
    //IP_recv(&pdu,&(socket1.local_addr.ip_addr), &(socket1.remote_addr.ip_addr),1000);
    int res=app_buffer_get(pdu.payload);
    if (pdu.header.ack_num==*PE || res==0){
        //creation pdu2+envoi ACK
    }




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


    app_buffer_put(pdu.payload);//on recup le message qui est dans les buffers

} 

