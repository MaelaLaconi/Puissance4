/*
	Canvas pour algorithmes de jeux à 2 joueurs
	
	joueur 0 : humain
	joueur 1 : ordinateur
			
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <limits.h>

// Paramètres du jeu
#define LARGEUR_MAX 7 		// nb max de fils pour un noeud (= nb max de coups possibles)
                            // on peut mettre un jeton sur chaque ligne

#define TEMPS 2		// temps de calcul pour un coup avec MCTS (en secondes)

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))

// Critères de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt {

	int joueur; // à qui de jouer ?

	// plateau pour le puissance_4
	char plateau[6][7];	

} Etat;

// Definition du type Coup
typedef struct {
	// un coup se definie par un couple (ligne, colonne)
	int ligne;
	int colonne;

} Coup;

// Copier un état 
Etat * copieEtat( Etat * src ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	etat->joueur = src->joueur;
	
		
	//copie de l'état src dans etat
	int i,j;	
	for (i=0; i< 6; i++)
		for ( j=0; j<7; j++)
			etat->plateau[i][j] = src->plateau[i][j];
	

	
	return etat;
}

// Etat initial 
Etat * etat_initial( void ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));
		
        // grille 6 lignes, 7 colonnes vides
        int i,j;	
	for (i=0; i< 6; i++)
		for ( j=0; j<7; j++)
			etat->plateau[i][j] = ' ';
	
	return etat;
}


void afficheJeu(Etat * etat) {

	/* par exemple : */
	int i,j;
	printf("   |");
	for ( j = 0; j < 7; j++) // nombre de colonnes
		printf(" %d |", j);
	printf("\n");
	printf("--------------------------------");
	printf("\n");
	
	for(i=0; i < 6; i++) { // nombre de lignes
		printf(" %d |", i);
		for ( j = 0; j < 7; j++) 
			printf(" %c |", etat->plateau[i][j]);
		printf("\n");
		printf("--------------------------------");
		printf("\n");
	}
}


// Nouveau coup 
Coup * nouveauCoup(int i, int j ) {
	Coup * coup = (Coup *)malloc(sizeof(Coup));
	
	// nouveau coup, couple (ligne, colonne)
    
	coup->ligne = i;
	coup->colonne = j;
	
	return coup;
}

// Demander à l'humain quel coup jouer 
Coup * demanderCoup (Etat * etat) {
    
    //demander seulement le numero de la colonne et calculer la ligne
	int ligne, col;

	printf(" quelle colonne ? ") ;
	scanf("%d",&col); 
    
  
    	// parcours de toutes les lignes de notre colonne
	for(ligne = 0 ; ligne < 6 ; ligne ++){
		// si la case est remplie on s'arrete pour avoir la derniere case vide
	    if(etat->plateau[ligne][col] != ' '){
		break ;
	    } 
    }
	

	return nouveauCoup(ligne - 1,col);
}

// Modifier l'état en jouant un coup 
// retourne 0 si le coup n'est pas possible
int jouerCoup( Etat * etat, Coup * coup ) {    
	
	// si on a deja un jeton a l'emplacement de notre coup
	if ( etat->plateau[coup->ligne][coup->colonne] != ' ' ){    
		return 0;
    	}else {
        
		etat->plateau[coup->ligne][coup->colonne] = etat->joueur ? 'O' : 'X';
		
		// à l'autre joueur de jouer
		etat->joueur = AUTRE_JOUEUR(etat->joueur); 	

		return 1;
	}	
}

// Retourne une liste de coups possibles à partir d'un etat 
// (tableau de pointeurs de coups se terminant par NULL)
Coup ** coups_possibles( Etat * etat ) {
	
	Coup ** coups = (Coup **) malloc((1+LARGEUR_MAX) * sizeof(Coup *) );
	
	int k = 0;
    
    	int ligne ;
    	for(int col = 0 ; col < 7 ; col ++){
	    for(ligne = 0 ; ligne < 6 ; ligne ++){
	        // on peut ajouter les jetons sur chaques colonnes, a la suite du dernier jeton mis sur celui-ci
	    	if ( etat->plateau[ligne][col] != ' ') { 
		    break ; // on sort de la boucle des qu'on a ajouté le coup associé a la colonne
		}
	    }
            if(ligne > 0){
                coups[k++] = nouveauCoup(ligne-1, col);
            }
        }
	/* fin de l'exemple */
	
	coups[k] = NULL;

	return coups;
}


// Definition du type Noeud 
typedef struct NoeudSt {
		
	int joueur; // joueur qui a joué pour arriver ici
	Coup * coup;   // coup joué par ce joueur pour arriver ici
	
	Etat * etat; // etat du jeu
			
	struct NoeudSt * parent; 
	struct NoeudSt * enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
	int nb_enfants;	// nb d'enfants présents dans la liste
	
	// POUR MCTS:
	int nb_victoires;
	int nb_simus;
	
} Noeud;


// Créer un nouveau noeud en jouant un coup à partir d'un parent 
// utiliser nouveauNoeud(NULL, NULL) pour créer la racine
Noeud * nouveauNoeud (Noeud * parent, Coup * coup ) {
	Noeud * noeud = (Noeud *)malloc(sizeof(Noeud));
	
	if ( parent != NULL && coup != NULL ) {
		noeud->etat = copieEtat ( parent->etat );
		jouerCoup ( noeud->etat, coup );
		noeud->coup = coup;			
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);		
	}
	else {
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0; 
	}
	noeud->parent = parent; 
	noeud->nb_enfants = 0; 
	
	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;	
	

	return noeud; 	
}

// Ajouter un enfant à un parent en jouant un coup
// retourne le pointeur sur l'enfant ajouté
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

void freeNoeud ( Noeud * noeud) {
	if ( noeud->etat != NULL)
		free (noeud->etat);
		
	while ( noeud->nb_enfants > 0 ) {
		freeNoeud(noeud->enfants[noeud->nb_enfants-1]);
		noeud->nb_enfants --;
	}
	if ( noeud->coup != NULL)
		free(noeud->coup); 

	free(noeud);
}
	

// Test si l'état est un état terminal 
// et retourne NON, MATCHNUL, ORDI_GAGNE ou HUMAIN_GAGNE
FinDePartie testFin( Etat * etat ) {

	
	// tester si un joueur a gagné
	int i,j,k,n = 0;
	for ( i=0;i < 6; i++) {
		for(j=0; j < 7; j++) {
			if ( etat->plateau[i][j] != ' ') {
				n++;	// nb coups joués
			
				// lignes
				k=0;
				while ( k < 4 && i+k < 6 && etat->plateau[i+k][j] == etat->plateau[i][j] ) 
					k++;
				if ( k == 4 ) // si on a 4 jetons d'affilé sur la meme ligne
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

				// colonnes
				k=0;
				while ( k < 4 && j+k < 7 && etat->plateau[i][j+k] == etat->plateau[i][j] ) 
					k++;
				if ( k == 4 ) // si on a 4 jetons d'affilé sur la meme colonne 
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

                
                
                
				// diagonales
				k=0;                            
				while ( k < 4 && i+k < 6 && j+k < 7 && etat->plateau[i+k][j+k] == etat->plateau[i][j] ) 
					k++;
                
                
				if ( k == 4 ) // si on a 4 jetons d'affilé sur la meme diagonale 
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

                
                
                
				k=0;
                            
				while ( k < 4 && i+k < 6 && j-k >= 0 && etat->plateau[i+k][j-k] == etat->plateau[i][j] ) 
					k++;
				if ( k == 4 ) // si on a 4 jetons d'affilé sur la meme diagonale
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;		
			}
		}
	}

	// et sinon tester le match nul	 (si on a parcourut toutes les cases cad si tout est remplit)
	if ( n == 6*7 ) 
		return MATCHNUL;
		
	return NON;
}

int strat ; // 0 pour max et 1 pour robuste
int optimisation ; // amelioration de la simulation (question 3)  ie toujours choisir un coup gagnant

// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes = 3
void ordijoue_mcts(Etat * etat, int tempsmax) {

	clock_t tic, toc;
	tic = clock();
	int temps;

	Coup ** coups;
        Coup ** coupsPossibles ;
	Coup * meilleur_coup ;
	
	// Créer l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);	
	racine->etat = copieEtat(etat); 
	
	// créer les premiers noeuds:
	coups = coups_possibles(racine->etat); 
    
	int k = 0;
	Noeud * enfant;
	while ( coups[k] != NULL) {
		enfant = ajouterEnfant(racine, coups[k]);
		k++;
	}
	
	

	int iter = 0;

    do {
	
        Noeud *currentNode = racine ; // l'etat courant
        Noeud *selectedNode, *listEnfants[LARGEUR_MAX] ;
        Noeud *maxNode ; //noeuds avec la Bvalue max
        int cptEnfants = 0 ; // index pour remplir le tableau d'enfant
        double bValue, Ui, max ; 
        int i = 0 ;
        int continuer = 1 ;
        
        // Sélectionner
        while(continuer == 1){
            max = INT_MIN ; // valeur par defaut
            
            // si on a pas un noeud terminal
            if(testFin(currentNode->etat)  == NON){

                for(i = 0 ; i < currentNode->nb_enfants ; i++){
                    enfant = currentNode->enfants[i] ;
                    
                    // si le noeud n'a jamais été simulé
                    if(enfant->nb_simus == 0){
                        // ajout du noeud au enfants pour simuler 
                        listEnfants[cptEnfants++] = enfant ;
                        continuer = 0 ;
                    }else{ // si on est dans le cas avec un noeud qui a été deja simulé
                        
                            // calcul de µi
                            Ui = (double) enfant->nb_victoires/(double) enfant->nb_simus ; 
                            
                            // signe pour la b value
                            int signe = 1 ;
                            
                            // si c'est a l'humain de jouer
                            if(enfant->joueur == 0){
                                signe = -1 ;
                            }
                            
                            // avec c = sqrt(2)
                            bValue = (signe * Ui) + sqrt(2) * sqrt(log(currentNode->nb_simus)/enfant->nb_simus) ;
                            
                            if(bValue > max){
                                // on remplace la valeur max des b value 
                                max = bValue ;
                                
                                // noeud avec la plus grande bValue
                                maxNode = enfant ;
                            }
                            
                    }
                }
                
                // cas ou on est pas passé dans le if avec un noeud sans simulation
                if(continuer == 1){
                    currentNode = maxNode ;
                    
                    // si le noeud n'a pas d'endants
                    if(currentNode->nb_enfants == 0){
                        coups = coups_possibles(currentNode->etat) ;
                        
                        k = 0 ;
                        while ( coups[k] != NULL) {
                            ajouterEnfant(currentNode, coups[k]);
                            k++;
                        }
                    }
                }
            }else{ // si testFin(currentNode->etat)  != NON
                    listEnfants[cptEnfants++] = currentNode ;
                    continuer = 0 ;
            }
        }
	
	
        // on prend un noeud au hasard dans la liste des enfants non simulés
        selectedNode = listEnfants[rand()%cptEnfants] ;
        
        // on prend comme état de départ l'état du noeuds séléctionné
        Etat *state = copieEtat(selectedNode->etat) ;
        
        // si on ne choisit pas l'optilisation
        if(optimisation == 0){
            //tant que l'etat n'est pas final
            while(testFin(state) == NON){
                // coup possible a partir de state
                coupsPossibles = coups_possibles(state) ;
                k = 0 ;//nombre de coups possibles
                while ( coupsPossibles[k] != NULL) {
                    k++;
                }
                
                // on prend au hasard un coup dans la liste des coups possibles
                Coup* selectedCoup = coupsPossibles[rand()%k] ;
                jouerCoup(state, selectedCoup) ;
            }
        }else{// si on a choisit l'amelioration
            // prochain coup, initialiser avec le coup du début
            Etat *nextState = copieEtat(state) ;
            // permet de savoir si le prochain etat est final ou non
            int isFinal ;
            
            //tant que l'etat n'est pas final
             while(testFin(state) == NON){
                 isFinal = 0 ; // pour le moment on ne sait pas si l'etat est final
                 
                // coup possible a partir de state
                coupsPossibles = coups_possibles(state) ;
                k = 0 ;//nombre de coups possibles
                while ( coupsPossibles[k] != NULL) {
                    k++;
                }
                
                //parcours de toute la liste de coups pour choisir un coup gagnant si cela est possible
                for(int i = 0 ; i < k ; i++){
                    // prochain coup, initialiser avec le coup du début
                    nextState = copieEtat(state) ;
                    jouerCoup(nextState, coupsPossibles[i]) ;
                    
                    // si ce coup mene a la victoire de l'ordinateur
                    if(testFin(nextState) == ORDI_GAGNE){
                        isFinal = 1 ;
                        jouerCoup(state, coupsPossibles[i]) ;
                        break ; //on arrete de parcourir la liste car on a un etat gagnant pour l'ordi
                    }
                }
                
                // si aucun coup menant a une victoire pour l'ordi n'a été trouvé
                if(isFinal == 0){
                    // on prend un coup au hasard dans la liste
                    Coup* selectedCoup = coupsPossibles[rand()%k] ;
                    jouerCoup(state, selectedCoup) ;
                }
                
                
            }
            
        }
        
        
        currentNode = selectedNode ; // on prend comme noeud courant le noeud qui a ete choisi
        
        // remonter a la racine (jusqu'a ce qu'on arrive a un noeud sans parent)
        while(currentNode != NULL){
            // si l'ordi gagne  et on considere q'un match nul est une partie de perdue
            if(testFin(state) == ORDI_GAGNE || testFin(state) == MATCHNUL){
                currentNode->nb_victoires++ ;
            }
            
            //on ajoute une simulation au noeud courant
            currentNode->nb_simus++ ;
            // pour remonter jusqu'a la racine
            currentNode = currentNode->parent ;
            
        }
        
        free(state) ;
		toc = clock(); 
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;
	} while ( temps < tempsmax );
	
	/* fin de l'algorithme  */ 
    
        int maxSim = 0 ;
        // pour tout les enfants de la racine
	for(int i = 0 ; i < racine->nb_enfants ; i++){
            int nombreSimus ;
        
	    // choix stratégie max
	    if(strat == 0){
	        nombreSimus = racine->enfants[i]->nb_simus ;
	    }
	    else{ //strategie robuste
	        nombreSimus = racine->enfants[i]->nb_victoires ;
	    }
        
            if(nombreSimus > maxSim){
                meilleur_coup = racine->enfants[i]->coup ;
                maxSim = racine->enfants[i]->nb_simus ;
            }
        }
	// Jouer le meilleur premier coup
	jouerCoup(etat, meilleur_coup );

	//QUESTION 1
	int nb_simu = racine->nb_simus;
	float proba = (float)racine->nb_victoires/(float)nb_simu;
	printf("Details du calcul : %d / %d == %f \n", nb_simu, racine->nb_victoires, proba);
	printf ("\nLe nombre de simulations réalisés  : %d\t \nEstimation de la probabilité de victoire de l'ordinateur : %f \n",nb_simu, proba);
	
	
	// Penser à libérer la mémoire :
	freeNoeud(racine);
	free (coups);
}

int main(void) {

	Coup * coup;
        Coup ** coupsPossibles ;
	FinDePartie fin;
	
	// initialisation
	Etat * etat = etat_initial(); 
	etat->joueur = -1;
	
	
	 do {
        
	    // Choisir qui commence : 
		printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
		scanf("%d", &(etat->joueur) );	
        
         } while (etat->joueur < 0 || etat->joueur > 1);




	 do {
             //Choisir la stratégie
    	     printf("Quelle stratégie choisir (0 : robuste, 1 : max) ? ");
	     scanf("%d", &strat );        
         } while (strat < 0 || strat > 1);


         do {
            // Choisir si on veut ou non la simulation améliorer
   	    printf("Prendre la stratégie améliorée ? (0 : non, 1 : oui) ? ");
	    scanf("%d", &optimisation );
          
         } while (optimisation < 0 || optimisation > 1);


    
	// boucle de jeu
	do {
		printf("\n");
        // affiche la grille
		afficheJeu(etat);
		if ( etat->joueur == 0 ) {
			// tour de l'humain
			
			do {
			    coup = demanderCoup(etat);
			} while ( !jouerCoup(etat, coup) );
									
		}
		else {
		    // tour de l'Ordinateur
		    ordijoue_mcts( etat, TEMPS );
			
		}
		
		fin = testFin( etat );
	}	while ( fin == NON ) ;

	printf("\n");
	afficheJeu(etat);
		
	if ( fin == ORDI_GAGNE )
		printf( "** L'ordinateur a gagné **\n");
	else if ( fin == MATCHNUL )
		printf(" Match nul !  \n");
	else
		printf( "** BRAVO, l'ordinateur a perdu  **\n");
	return 0;
}
