/*
	Canvas pour algorithmes de jeux à 2 joueurs
	
	joueur 0 : humain
	joueur 1 : ordinateur
			
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Paramètres du jeu
#define LARGEUR_MAX 7 		// nb max de fils pour un noeud (= nb max de coups possibles)
                            // on peut mettre un jeton sur chaque ligne

#define TEMPS 3		// temps de calcul pour un coup avec MCTS (en secondes)

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
    
    // on prend pour ligne la dernière ligne vide (en partant du haut)
    ligne = 0 ;
       
    while(ligne < 6){
        // on se stop des qu'on a une case non vide (la derniere case vide devient notre num de ligne)
        if(etat->plateau[ligne][col] != ' '){
            break ;
        }
        ligne ++ ;
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
    
    
    for(int col = 0 ; col < 7 ; col ++){
        for(int ligne = 0 ; ligne < 6 ; ligne ++){
            // on peut ajouter les jetons sur chaques colonnes, a la suite du dernier jeton mis sur celui-ci
            if ( etat->plateau[ligne][col] != ' ' && ligne >=1) { 
				coups[k] = nouveauCoup(ligne - 1 ,col); // on ajoute le coup
				k++;
                break ; // on sort de la boucle des qu'on a ajouté le coup associé a la colonne
			}
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

                
                
                
				// diagonales : PAS SURE DES CALCULS
				k=0;                            // prendre 7 ou 6 ?
				while ( k < 4 && i+k < 6 && j+k < 7 && etat->plateau[i+k][j+k] == etat->plateau[i][j] ) 
					k++;
                
                
				if ( k == 4 ) // si on a 4 jetons d'affilé sur la meme diagonale 
					return etat->plateau[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;

                
                
                
				k=0;
                                    // prendre 6 ou 7 ?
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



// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes = 3
void ordijoue_mcts(Etat * etat, int tempsmax) {

	clock_t tic, toc;
	tic = clock();
	int temps;

	Coup ** coups;
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
	
	
	/*meilleur_coup = coups[ rand()%k ]; // choix aléatoire
	
	  TODO :
		- supprimer la sélection aléatoire du meilleur coup ci-dessus
		- implémenter l'algorithme MCTS-UCT pour déterminer le meilleur coup ci-dessous*/

	int iter = 0;
	//https://www.geeksforgeeks.org/ml-monte-carlo-tree-search-mcts/ base sur ca
	do {
	
        Noeud *currentState = racine ; // l'etat courant
        Noeud *resultSimulation = racine ;

        double bValue = 0 ; 
        int t = 0 ; // nombre total de simulations
        
        // tant que le noeud n'est pas terminal
        while(resultSimulation->nb_enfants != 0){
            // on prend un enfant aleatoirement
            resultSimulation =  resultSimulation->enfants[rand()%resultSimulation->nb_enfants] ; // -1 ou pas ??
        }
	
	
		// à compléter par l'algorithme MCTS-UCT... 
	
	
	
	
		toc = clock(); 
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;
	} while ( temps < tempsmax );
	
	/* fin de l'algorithme  */ 
	
	// Jouer le meilleur premier coup
	jouerCoup(etat, meilleur_coup );
	
	// Penser à libérer la mémoire :
	freeNoeud(racine);
	free (coups);
}

int main(void) {

	Coup * coup;
	FinDePartie fin;
	
	// initialisation
	Etat * etat = etat_initial(); 
	
	// Choisir qui commence : 
	printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
	scanf("%d", &(etat->joueur) );
	
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
