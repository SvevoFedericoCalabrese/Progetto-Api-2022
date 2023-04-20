
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *goalWord; //parola da indovinare
char *res;
char *resConstraints;
int k=0;
int globalCounter;        //per contare le parole compatibili coi vincoli
int *occurrencesArray;
char *strPerCP;
int ended = 0;

/*
 * La struttura dati utilizzata è un bst con una lista doppiamente linkata interna, infatti ogni nodo dell'albero ha un puntatore
 * al diretto successore(e predecessore) che ha la parola ancora compatibile con i vincoli appresi, inizialmente la lista contiene
 * tutti i nodi dell'albero
 */
struct treeNode {
    struct treeNode *left;
    struct treeNode *right;
    struct treeNode *father;
    struct treeNode *compatibleSuccessor;
    struct treeNode *compatiblePredecessor;
    char compatible;
    char word[];

};

struct treeNode *root,*min;

struct letterValue {
    char letter;
    int value;
    char maxReached;
    int n_exact;
    int n_min; //pos 0 per il minimo attuale pos 1 per il minimo eventuale da confrontare con l'attuale
    int pos[];
};

struct letterValue *letterValueArray[124];

int hashFunctionLetteraValore(char lettera) {
    return lettera;
}

/*
 * questa funzione permette di salvare per ogni letter tutte le informazioni che vengono apprese giocando
 * come la posizione nella stringa dove non può comparire, il numero minimo, massimo ed esatto delle occorrenze 
 * di quella lettera nella parola da indovinare
 */
void insertInLetteraValore(char lettera, int valore,int pos,int n_max, int n_min ) {
    struct letterValue *item;
    if (letterValueArray[hashFunctionLetteraValore(lettera)] == NULL) {
        item = malloc(sizeof item[0] + sizeof item->pos[0] * (k+1));

        item->letter = lettera;
        item->value = valore;

        for (int i = 0; i < k; i++)
            item->pos[i] = -1;
        item->maxReached = '0';
        item->n_exact = n_max;
        item->n_min = n_min;
        letterValueArray[hashFunctionLetteraValore(lettera)] =  item;
    }
    else {
        letterValueArray[hashFunctionLetteraValore(lettera)]->value = valore;
        letterValueArray[hashFunctionLetteraValore(lettera)]->letter = lettera;
        for (int i = 0; i < k; i++)
            letterValueArray[hashFunctionLetteraValore(lettera)]->pos[i] = -1;
        letterValueArray[hashFunctionLetteraValore(lettera)]->maxReached = '0';
        letterValueArray[hashFunctionLetteraValore(lettera)]->n_exact = n_max;
        letterValueArray[hashFunctionLetteraValore(lettera)]->n_min = n_min;

    }
}
struct treeNode* getMin(struct treeNode *node)
{
    while (node->left != NULL) {
        node = node->left;
    }
    return node;
}


struct treeNode* inOrderSuccessor(struct treeNode *n)
{
    if (n->right != NULL)
        return getMin(n->right);

    struct treeNode* p = n->father;
    while (p != NULL && n == p->right) {
        n = p;
        p = p->father;
    }
    return p;
}


/*
 * questa funzione,prende in ingresso un nodo qualsiasi dell'albero e
 * tramite una ricerca inOrder trova il primo successore la cui la parola è ancora compatibile coi vincoli appresi
 */
struct treeNode* compatibleSuccessor(struct treeNode *n)
{

    struct treeNode* current = n;
        while (inOrderSuccessor(current) != NULL ) {//todo corner case
            current = inOrderSuccessor(current);
            if (current->compatible == '1')
                return current;
        }
    return NULL;
}

/*
 * come prima ma per il predecessore
 */
struct treeNode* compatiblePredecessor(struct treeNode *n)
{
    struct treeNode *curr,*prev;
    curr = min;
    prev = min;
    if (compatibleSuccessor(curr) != NULL) {
        while (compatibleSuccessor(curr) != NULL) {
            if (curr->compatible == '1')
                prev = curr;
            curr = compatibleSuccessor(curr);

        }
        prev->compatibleSuccessor = n;
        return prev;
    }
        min->compatibleSuccessor = n;
        return min;
}

/*
 * costruisce la lista interna,collegando i puntatori dei nodi nel modo corretto
 */
void fillSuccessorField() {
    struct treeNode* current = getMin(root);
    struct treeNode* temp;
    current->compatiblePredecessor = NULL;
    while (current != NULL){
        current->compatibleSuccessor = inOrderSuccessor(current);
        if (current->compatibleSuccessor == NULL)  {

            break;
        }

        temp = current;
        current = current->compatibleSuccessor;
        current->compatiblePredecessor = temp;

    }
    current->compatibleSuccessor = NULL;
}

int countLetters(char *string, char c) {
    int counter = 0;
    for (int i = 0; i < k; i++) {
        if (c == string[i])
            counter++;
    }
    return counter;
}

int countLetterInCorrectPos(char *string, char c, char *r) {
    int counter = 0;
    for (int i = 0; i < k; i++) {
        if (c == string[i] && r[i] == c)
            counter++;
    }
    return counter;
}

void insertInPosArray(int posArray[],int value) {
    for (int i = 0; i < k; i++) {
        if (posArray[i] == -1) {
            posArray[i] = value;
            break;
        }

    }
}

int searchArray(int posArray[],int pos) {
    for (int i = 0; i < k; i++) {
        if (posArray[i] > -1) {
            if (posArray[i] == pos)
                return 1;
        }
        else
            break;
    }
    return 0;
}

/*
 * prende un nodo in input e controlla che la parola contenuta nel nodo sia ancora compatibile, sostanzialmente controlla che ogni
 * letter della parola rispetti tutti i vincoli salvati in "letterValueArray", nel caso la parola non sia compatibile setta
 *il campo "compatible" a 0
 *
 * questa funzione è utilizzata durante la partita per segnare i nodi che non sono più disponibili dopo che la parola giocata
 * viene letta in input, la funzione è chiamata per ogni nodo
 * */
int filterSingleWord(struct treeNode *node) {
    char l;
    int exitCicle = 0;

    int nlettere = 0;
    if (node->compatible != '0') {
        for (int i = 0; (i < k && exitCicle != 1); i++) {
            l = node->word[i];
            if (letterValueArray[hashFunctionLetteraValore(l)]->value != -1) {
                if (letterValueArray[hashFunctionLetteraValore(l)]->value ==
                    0) {
                    node->compatible = '0';
                    globalCounter--;
                    exitCicle = 1;
                    break;

                } else if (searchArray(letterValueArray[hashFunctionLetteraValore(l)]->pos, i) == 1) {
                    node->compatible = '0';
                    globalCounter--;
                    exitCicle = 1;
                    break;
                }
            }
            if (resConstraints[i] != '*') {
                if (node->word[i] !=
                    goalWord[i]) {
                    node->compatible = '0';
                    globalCounter--;
                    exitCicle = 1;
                    break;
                }
            }
            if (occurrencesArray[i] != 0)
                nlettere = countLetters(node->word, goalWord[i]);
            if (occurrencesArray[i] > 0 && nlettere < occurrencesArray[i]) {
                node->compatible = '0';
                globalCounter--;
                exitCicle = 1;
                break;
            }
            if (occurrencesArray[i] < 0 && nlettere != occurrencesArray[i] * -1) {
                node->compatible = '0';
                globalCounter--;
                exitCicle = 1;
                break;
            }

        }
    }

    return exitCicle;
}

/*
 * scorre tutta la lista delle parole ancora compatibili(quindi non l'intero albero!) ed elimina da questa le parole non più compatibili
 */
void filterDictionary(struct treeNode* root) {

    struct treeNode *current;
    struct treeNode *prev;
    struct treeNode *temp;

    while (filterSingleWord(min) == 1)
        min = min->compatibleSuccessor;

    if (min->compatibleSuccessor != NULL) {
        prev = min;
        current = min->compatibleSuccessor;
        current->compatiblePredecessor = min;
        while (current->compatibleSuccessor != NULL) {

            if (filterSingleWord(current) == 1) {
                prev->compatibleSuccessor = current->compatibleSuccessor;
                (current->compatibleSuccessor)->compatiblePredecessor = prev;

            } else
                prev = current;

            current = current->compatibleSuccessor;
        }

        if (filterSingleWord(current) == 1) {
            temp = current->compatiblePredecessor;
            temp->compatibleSuccessor = NULL;
        }
    }
}

/*
 * conofronta la parola giocata con quella da indovinare e aggiorna i vincoli sulle lettere in "letterValueArray" di conseguenza
 */
char *compareWords(char *s1, char *s2) {
    int  n = 0,c = 0,counter = 0;
    for (int i = 0; i < k; i++) {

        if(s2[i] == s1[i]) {
            strPerCP[i] = '+';
            resConstraints[i]=s1[i];
            letterValueArray[hashFunctionLetteraValore(s2[i])]->value =
                    letterValueArray[hashFunctionLetteraValore(s2[i])]->value + 2;

            letterValueArray[hashFunctionLetteraValore(s2[i])]->n_min++;

        } else if (countLetters(s1, s2[i]) == 0) {

            strPerCP[i] = '/';
            letterValueArray[hashFunctionLetteraValore(s2[i])]->value = 0;


        }else if (countLetters(s1, s2[i]) != 0 && s2[i] != s1[i]  ) {


            n = countLetters(s1, s2[i]);
            c = countLetterInCorrectPos(s2, s2[i], s1);
            if (n > 0) {
                for (int j = 0; j < i; j++) {
                    if (s2[j] == s2[i] && s2[j] != s1[j])
                        counter++;
                }
                if (counter >= n-c ) {
                    strPerCP[i] = '/';

                    letterValueArray[hashFunctionLetteraValore(s2[i])]->n_exact = n;
                    letterValueArray[hashFunctionLetteraValore(s2[i])]->maxReached = '1';

                    insertInPosArray(letterValueArray[hashFunctionLetteraValore(s2[i])]->pos, i);
                    letterValueArray[hashFunctionLetteraValore(s2[i])]->value =
                            letterValueArray[hashFunctionLetteraValore(s2[i])]->value + 2;


                }else {


                    letterValueArray[hashFunctionLetteraValore(s2[i])]->n_min++;
                    strPerCP[i] = '|';

                    insertInPosArray(letterValueArray[hashFunctionLetteraValore(s2[i])]->pos, i);

                    /**
                     * Conto solo le lettere che so essere presenti(in base alle parole "giocate") nella word di riferimento ma che sono in posizione sbagliata
                     */
                    letterValueArray[hashFunctionLetteraValore(s2[i])]->value =
                            letterValueArray[hashFunctionLetteraValore(s2[i])]->value + 2;

                }
            }
        }
        counter = 0;

    }

    for (int i = 0; i < k; i++) {
        if (letterValueArray[hashFunctionLetteraValore(s1[i])]->maxReached == '0' && occurrencesArray[i] < letterValueArray[hashFunctionLetteraValore(s1[i])]->n_min)
            occurrencesArray[i] = letterValueArray[hashFunctionLetteraValore(s1[i])]->n_min;


        if (letterValueArray[hashFunctionLetteraValore(s1[i])]->maxReached == '1')
            occurrencesArray[i] = -1 * letterValueArray[hashFunctionLetteraValore(s1[i])]->n_exact;

    }
    for (int i = 0; i < k; i++) {
        letterValueArray[hashFunctionLetteraValore(s1[i])]->n_min = 0;
    }


    filterDictionary(root);
    return strPerCP;
}


int strComparator(char *s1, char *s2){
    while (*s1 != '\0') {
        if (*s1 < *s2)
            return -1;
        if (*s1 > *s2)
            return 1;
        s1++;
        s2++;
    }
    return 0;
}

void inOrderPrint(struct  treeNode *node) {
    node = min;
    while (node){
        if (node->compatible == '1')
            printf("%s\n", node->word);
        node = node->compatibleSuccessor;
    }

}


int inOrderResearch(char *word, struct treeNode *node) {
    while (node != NULL) {
        if (strComparator(node->word, word) > 0) {
            node = node->left;
        } else if (strComparator(node->word, word) < 0) {
            node = node->right;
        }
        else if(strcmp(node->word, word) == 0 )
            return 1;
    }
    return 0;

}

void restoreTree(struct treeNode *root) {
    if (root == NULL) {
        return;
    }
    restoreTree(root->left);
    if (root->compatible == '0') {
        root->compatible = '1';
        globalCounter++;
    }
    restoreTree(root->right);
}



char *customStrCopy(char *dest, int size, char *src) {
    int i;
    for (i = 0; i < size ; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';

    return dest;
}

/*
 * Inserisce ogni nuovo nodo nell'albero, ma se viene inserito durante una partita viene controllato che la goalWord sia compatibile, se si il nodo
 * viene aggiunto nella lista (in ogni caso viene aggiunto all'albero perchè la goalWord va ad arrichire il dizionario che sarà usato anche nelle successive partite)
 *
 * Se il nodo è aggiunto prima o dopo una partita viene semplicemente aggiunto all'albero e non alla lista, questa verrà poi sistema dalla funzione "fillSuccessorTree"
 */
void insert(char *word, char compatible, char inGame) {//TODO
    struct treeNode *currentNode;
    struct treeNode *new_node;
    struct treeNode *parent;
    struct treeNode *temp,*temp2;

    currentNode = root;
    parent = NULL;
    int trovato = 0;
    while (currentNode != NULL && trovato == 0) {
        parent = currentNode;
        if (strComparator(currentNode->word, word) > 0) {
            currentNode = currentNode->left;
        } else if (strComparator(currentNode->word, word) < 0) {
            currentNode = currentNode->right;
        }
        else
            trovato = 1;
    }

    new_node = malloc(sizeof new_node[0] + sizeof new_node->word[0] * (k + 1));
    if (new_node) {
        customStrCopy(new_node->word, k, word);

        new_node->left = NULL;
        new_node->compatibleSuccessor = NULL;
        new_node->compatiblePredecessor = NULL;//TODO
        new_node->right = NULL;
        new_node->compatible = compatible;
    }

    if (parent == NULL) {
        root = new_node;

    } else if (parent != NULL && strComparator(parent->word, word) > 0) {
        parent->left = new_node;
    } else if (parent != NULL && strComparator(parent->word, word) < 0) {
        parent->right = new_node;
    }
    new_node->father = parent;

    if (inGame == '0')
        globalCounter++;

    else if (inGame == '1' && compatible == '1'){
        globalCounter++;

        if (strcmp(new_node->word, min->word) < 0) {
            new_node->compatibleSuccessor = min;
            new_node->compatiblePredecessor = NULL;
            min->compatiblePredecessor = new_node;
            min = new_node;

        }else {
            temp = compatibleSuccessor(new_node);
            new_node->compatibleSuccessor = temp;
            if (temp != NULL) {
                new_node->compatiblePredecessor = temp->compatiblePredecessor;
                if (temp->compatiblePredecessor != NULL) {
                    temp2 = temp->compatiblePredecessor;
                    temp->compatiblePredecessor = new_node;
                    temp2->compatibleSuccessor = new_node;
                }else
                    temp->compatiblePredecessor = new_node;

            } else {
                new_node->compatiblePredecessor = compatiblePredecessor(new_node);
                new_node->compatibleSuccessor = NULL;
            }
        }
    }
}

/*
 * se una parola viene aggiunta al dizionario durante una partita, questa funzione controlla se tale parole rispetta i vincoli trovati
 */
char filterWord (char *p, char *r) {
    char returnValue = '1';
    int exitCicle = 0;
    char l;

    for (int i = 0; (i < k && exitCicle != 1); i++) {
        l = p[i];

        if (letterValueArray[hashFunctionLetteraValore(l)]->value != -1) {

            if (letterValueArray[hashFunctionLetteraValore(l)]->value ==
                0) {
                returnValue = '0';

                exitCicle = 1;
                break;

            } else if (searchArray(letterValueArray[hashFunctionLetteraValore(l)]->pos, i) == 1) {
                returnValue = '0';
                exitCicle = 1;
                break;
            }
        }
        if (resConstraints[i] != '*') {
            if (p[i] !=
                goalWord[i]) {
                returnValue = '0';
                exitCicle = 1;
                break;
            }
        }
        if (occurrencesArray[i] > 0 && countLetters(p, goalWord[i]) < occurrencesArray[i]) {
            returnValue = '0';
            exitCicle = 1;
            break;
        }
        if (occurrencesArray[i] < 0 && countLetters(p, goalWord[i]) != (occurrencesArray[i] * -1)) {
            returnValue = '0';

            exitCicle = 1;
            break;
        }


    }
    return returnValue;
}
int hasWon(int k,char *r ) {
    int ok = 0;
    for (int i = 0; i<k; i++) {
        if (res[i] == '+')
            ok++;
        else
            ok = 0;
    }
    if (ok >= k) {

        return 1;

    }
    return 0;

}

void startGame() {
    char *str;
    int n = 0;

    char *getNum = malloc(sizeof(char) *1000);
    str = malloc(sizeof(char) * (k+20));
    goalWord = malloc(sizeof(char) * (k + 2));
    res = malloc(sizeof(char) * (k+2));

    for (int i = 0; i < k; i++)
        res[i] = '/';
    res[k] = '\0';

    if (fgets(goalWord, k + 20, stdin) != NULL)
        goalWord[strcspn(goalWord, "\n")] = 0;


    if (fgets(getNum,1000, stdin) != NULL)
        getNum[strcspn(getNum, "\n")] = 0;
    n = atoi(getNum);

    while (!feof(stdin)) {

        if (fgets(str, k+20, stdin) !=NULL)
            str[strcspn(str, "\n")] = 0;

        if (strcmp(str, "+stampa_filtrate\0") == 0) {
            inOrderPrint(root);

        }else if (strcmp(str, "+inserisci_inizio\0") == 0) {

            if (fgets(str, k+20, stdin) != NULL)
                str[strcspn(str, "\n")] = 0;
            while (strcmp("+inserisci_fine\0", str) != 0) {


                if (ended == 0) {
                    if (str[0]!='+')
                        insert(str, filterWord(str, resConstraints), 1);
                }else {
                    if (str[0]!='+')
                        insert(str, 1, 0);
                }
                if (fgets(str,k+20, stdin) != NULL)
                    str[strcspn(str, "\n")] = 0;
            }

        } else if (strcmp("+nuova_partita\0", str) == 0) {

            if (fgets(goalWord, k + 20, stdin) != NULL)
                goalWord[strcspn(goalWord, "\n")] = 0;


            if (fgets(getNum,1000, stdin) != NULL)
                getNum[strcspn(getNum, "\n")] = 0;
            n = atoi(getNum);

            restoreTree(root);
            min = getMin(root);
            fillSuccessorField();

            for (int i = 65; i <= 90; i++)
                insertInLetteraValore(i,-1,0,0,0);
            for (int i = 97; i <= 122; i++)
                insertInLetteraValore(i,-1,0,0,0);
            for (int i = 48; i <= 57; i++)
                insertInLetteraValore(i,-1,0,0,0);
            insertInLetteraValore('-',-1,0,0,0);
            insertInLetteraValore('_',-1,0,0,0);
            for(int i = 0; i < k; i++)
                occurrencesArray[i] = 0;

            for (int i = 0; i < k; i++)
                res[i] = '/';
            res[k] = '\0';

            for (int i = 0; i < k; i++)
                resConstraints[i] = '*';
            resConstraints[k] = '\0';

            ended = 0;



        } else if ((str[0] != '+') && inOrderResearch(str, root) == 0) {
            printf("not_exists\n");

        } else if (str[0] != '+'){
            res = compareWords(goalWord, str);
            if (hasWon(k, res) != 1 && ended == 0)
                printf("%s\n%d\n", res, globalCounter);
            n--;

            if (n <= 0 && hasWon(k, res) != 1 && ended == 0) {
                printf("ko\n");
                ended = 1;
            } else if (hasWon(k, res) == 1 && ended == 0) {
                printf("ok\n");
                ended = 1;
            }
        }
    }
}

void fillTree() {

    char *getNum = malloc(sizeof(char)*(1000));

    if (fgets(getNum,1000, stdin) != NULL)
        getNum[strcspn(getNum, "\n")] = 0;
    k = atoi(getNum);

    char *newWord = malloc(sizeof(char) * (k + 20));

    occurrencesArray = malloc(sizeof (int) * (k + 2));
    for(int i = 0; i < k; i++)
        occurrencesArray[i] = 0;

    resConstraints = malloc(sizeof(char) * (k + 2));

    for (int i = 0; i < k; i++)
        resConstraints[i] = '*';
    resConstraints[k] = '\0';

    strPerCP = malloc(sizeof(char)* (k+2));
    for (int i = 0; i < k; i++)
        strPerCP[i] = '/';
    strPerCP[k] = '\0';



    for (int i = 65; i <= 90; i++)
        insertInLetteraValore(i,-1,0,0,0);
    for (int i = 97; i <= 122; i++)
        insertInLetteraValore(i,-1,0,0,0);
    for (int i = 48; i <= 57; i++)
        insertInLetteraValore(i,-1,0,0,0);
    insertInLetteraValore('-',-1,0,0,0);
    insertInLetteraValore('_',-1,0,0,0);


    if (fgets (newWord, k + 20, stdin) != NULL) {
        newWord [strcspn(newWord, "\n")] = 0;
        while( (strcmp("+nuova_partita\0", newWord) != 0)) {
            if (newWord[0] != '+')
                insert(newWord, 1, 0);
            if (fgets(newWord, k + 20, stdin) != NULL)
                newWord[strcspn(newWord, "\n")] = 0;

        }
        min = getMin(root);
        fillSuccessorField();

    }

}

int main() {
    globalCounter = 0;
    fillTree();
    startGame();
    return 0;
}








