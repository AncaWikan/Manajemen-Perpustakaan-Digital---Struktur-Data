#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_TITLE 100
#define MAX_NAME 50
#define MAX_STACK 100
#define HASH_SIZE 101

typedef struct Buku {
    int id;
    char judul[MAX_TITLE];
    char penulis[MAX_NAME];
    int tahun;
    char kategori[50];
    bool tersedia;
    struct Buku *next;
} Buku;

typedef struct Pengguna {
    int id;
    char nama[MAX_NAME];
} Pengguna;

typedef struct Peminjaman {
    int idUser;
    int idBuku;
    char aksi[10];
} Peminjaman;

/* Stack */
Peminjaman undoStack[MAX_STACK];
int top = -1;

void pushUndo(Peminjaman p) {
    if (top < MAX_STACK - 1)
        undoStack[++top] = p;
}

Peminjaman popUndo() {
    if (top >= 0)
        return undoStack[top--];
    Peminjaman kosong = {-1, -1, ""};
    return kosong;
}

/* Queue */
typedef struct QueueNode {
    int idUser;
    struct QueueNode *next;
} QueueNode;

typedef struct Queue {
    QueueNode *front, *rear;
} Queue;

void initQueue(Queue *q) {
    q->front = q->rear = NULL;
}

void enqueue(Queue *q, int idUser) {
    QueueNode *newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->idUser = idUser;
    newNode->next = NULL;
    if (!q->rear)
        q->front = q->rear = newNode;
    else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

int dequeue(Queue *q) {
    if (!q->front) return -1;
    QueueNode *temp = q->front;
    int id = temp->idUser;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    free(temp);
    return id;
}

void clearQueue(Queue *q) {
    while (q->front) {
        QueueNode *temp = q->front;
        q->front = q->front->next;
        free(temp);
    }
    q->rear = NULL;
}

/* AVL Tree */
typedef struct AVLNode {
    Buku data;
    struct AVLNode *left, *right;
    int height;
    Queue antrean;
} AVLNode;

int height(AVLNode *node) {
    return node ? node->height : 0;
}

int getBalance(AVLNode *node) {
    return node ? height(node->left) - height(node->right) : 0;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

AVLNode* newAVLNode(Buku data) {
    AVLNode *node = (AVLNode*)malloc(sizeof(AVLNode));
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1;
    initQueue(&node->antrean);
    return node;
}

AVLNode* rightRotate(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

AVLNode* leftRotate(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

AVLNode* insertAVL(AVLNode* node, Buku data) {
    if (!node) return newAVLNode(data);
    if (data.id < node->data.id)
        node->left = insertAVL(node->left, data);
    else if (data.id > node->data.id)
        node->right = insertAVL(node->right, data);
    else return node;

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = getBalance(node);

    if (balance > 1 && data.id < node->left->data.id) return rightRotate(node);
    if (balance < -1 && data.id > node->right->data.id) return leftRotate(node);
    if (balance > 1 && data.id > node->left->data.id) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && data.id < node->right->data.id) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

AVLNode* findBook(AVLNode *root, int id) {
    if (!root || root->data.id == id) return root;
    return (id < root->data.id) ? findBook(root->left, id) : findBook(root->right, id);
}

void inorderTraversal(AVLNode *root) {
    if (root) {
        inorderTraversal(root->left);
        printf("ID: %d | Judul: %s | Penulis: %s | Tersedia: %s\n",
               root->data.id, root->data.judul, root->data.penulis,
               root->data.tersedia ? "Ya" : "Tidak");
        inorderTraversal(root->right);
    }
}

void freeAVL(AVLNode *root) {
    if (root) {
        freeAVL(root->left);
        freeAVL(root->right);
        clearQueue(&root->antrean);
        free(root);
    }
}

/* Hash Table */
typedef struct UserHashNode {
    Pengguna user;
    struct UserHashNode *next;
} UserHashNode;

UserHashNode* userTable[HASH_SIZE] = {NULL};

int hash(int key) {
    return key % HASH_SIZE;
}

void insertUser(Pengguna user) {
    int index = hash(user.id);
    UserHashNode *newNode = (UserHashNode*)malloc(sizeof(UserHashNode));
    newNode->user = user;
    newNode->next = userTable[index];
    userTable[index] = newNode;
}

Pengguna* findUser(int id) {
    int index = hash(id);
    UserHashNode *current = userTable[index];
    while (current) {
        if (current->user.id == id) return &current->user;
        current = current->next;
    }
    return NULL;
}

/* Input Buku & Pengguna */
void inputBuku(AVLNode **root) {
    Buku b;
    printf("\nMasukkan ID Buku: "); scanf("%d", &b.id); getchar();
    printf("Masukkan Judul Buku: "); fgets(b.judul, MAX_TITLE, stdin); b.judul[strcspn(b.judul, "\n")] = 0;
    printf("Masukkan Penulis Buku: "); fgets(b.penulis, MAX_NAME, stdin); b.penulis[strcspn(b.penulis, "\n")] = 0;
    printf("Masukkan Tahun Terbit: "); scanf("%d", &b.tahun); getchar();
    printf("Masukkan Kategori: "); fgets(b.kategori, 50, stdin); b.kategori[strcspn(b.kategori, "\n")] = 0;
    b.tersedia = true;
    *root = insertAVL(*root, b);
}

void inputPengguna() {
    Pengguna u;
    printf("\nMasukkan ID Pengguna: "); scanf("%d", &u.id); getchar();
    printf("Masukkan Nama Pengguna: "); fgets(u.nama, MAX_NAME, stdin); u.nama[strcspn(u.nama, "\n")] = 0;
    insertUser(u);
}

/* Peminjaman */
void pinjamBuku(AVLNode *root) {
    int idUser, idBuku;
    printf("Masukkan ID Pengguna: "); scanf("%d", &idUser);
    printf("Masukkan ID Buku: "); scanf("%d", &idBuku);
    Pengguna *u = findUser(idUser);
    if (!u) { printf("Pengguna tidak ditemukan!\n"); return; }
    AVLNode *b = findBook(root, idBuku);
    if (!b) { printf("Buku tidak ditemukan!\n"); return; }
    if (b->data.tersedia) {
        b->data.tersedia = false;
        pushUndo((Peminjaman){idUser, idBuku, "pinjam"});
        printf("Peminjaman berhasil.\n");
    } else {
        enqueue(&b->antrean, idUser);
        printf("Buku sedang dipinjam. Ditambahkan ke antrean (Posisi: %d).\n", 
               b->antrean.rear ? b->antrean.rear->idUser : idUser);
    }
}

void kembalikanBuku(AVLNode *root) {
    int idUser, idBuku;
    printf("Masukkan ID Pengguna: "); scanf("%d", &idUser);
    printf("Masukkan ID Buku: "); scanf("%d", &idBuku);
    AVLNode *b = findBook(root, idBuku);
    if (!b) { printf("Buku tidak ditemukan!\n"); return; }
    if (!b->data.tersedia) {
        b->data.tersedia = true;
        pushUndo((Peminjaman){idUser, idBuku, "kembali"});
        if (b->antrean.front) {
            int nextUser = dequeue(&b->antrean);
            b->data.tersedia = false;
            pushUndo((Peminjaman){nextUser, idBuku, "pinjam"});
            printf("Buku otomatis dipinjam oleh ID %d\n", nextUser);
        } else {
            printf("Pengembalian berhasil.\n");
        }
    } else printf("Buku sudah tersedia.\n");
}

/* Fungsi Undo yang Diperbaiki */
void undoTerakhir(AVLNode *root) {
    if (top == -1) {
        printf("Tidak ada aksi untuk di-undo.\n");
        return;
    }

    Peminjaman p = popUndo();
    AVLNode *b = findBook(root, p.idBuku);
    if (!b) {
        printf("Buku tidak ditemukan.\n");
        return;
    }

    if (strcmp(p.aksi, "pinjam") == 0) {
        // Undo peminjaman - kembalikan buku
        b->data.tersedia = true;
        printf("Undo: Buku ID %d dikembalikan oleh ID %d.\n", p.idBuku, p.idUser);
        
        // Jika ada antrian, beri opsi untuk meminjamkan ke antrian berikutnya
        if (b->antrean.front) {
            printf("Antrian tersedia:\n");
            QueueNode *current = b->antrean.front;
            while (current) {
                Pengguna *u = findUser(current->idUser);
                if (u) printf("- ID: %d | Nama: %s\n", u->id, u->nama);
                current = current->next;
            }
            
            printf("Apakah ingin meminjamkan ke antrian terdepan? (y/n): ");
            char choice;
            scanf(" %c", &choice);
            if (choice == 'y' || choice == 'Y') {
                int nextUser = dequeue(&b->antrean);
                b->data.tersedia = false;
                pushUndo((Peminjaman){nextUser, p.idBuku, "pinjam"});
                printf("Buku dipinjamkan ke ID %d\n", nextUser);
            }
        }
    } 
    else if (strcmp(p.aksi, "kembali") == 0) {
        // Undo pengembalian - pinjam kembali buku
        b->data.tersedia = false;
        printf("Undo: Buku ID %d dipinjam kembali oleh ID %d.\n", p.idBuku, p.idUser);
    }
}

/* Sorting */
int treeToArray(AVLNode *root, Buku arr[], int index) {
    if (!root) return index;
    index = treeToArray(root->left, arr, index);
    arr[index++] = root->data;
    index = treeToArray(root->right, arr, index);
    return index;
}

void sortByJudul(Buku arr[], int n) {
    for (int i = 0; i < n-1; ++i)
        for (int j = 0; j < n-i-1; ++j)
            if (strcmp(arr[j].judul, arr[j+1].judul) > 0) {
                Buku temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
}

void tampilkanBukuTerurut(AVLNode *root) {
    Buku daftar[100];
    int n = treeToArray(root, daftar, 0);
    sortByJudul(daftar, n);
    printf("\nDaftar Buku Terurut Berdasarkan Judul:\n");
    for (int i = 0; i < n; ++i)
        printf("ID: %d | Judul: %s | Penulis: %s | Tahun: %d | Tersedia: %s\n",
               daftar[i].id, daftar[i].judul, daftar[i].penulis, daftar[i].tahun,
               daftar[i].tersedia ? "Ya" : "Tidak");
}

/* Main */
int main() {
    AVLNode *root = NULL;
    int pilihan;
    do {
        printf("\n===== Menu Perpustakaan =====\n");
        printf("1. Tambah Buku\n2. Tambah Pengguna\n3. Lihat Daftar Buku\n");
        printf("4. Cari Pengguna\n5. Pinjam Buku\n6. Kembalikan Buku\n");
        printf("7. Undo Terakhir\n8. Tampilkan Antrean Buku\n");
        printf("9. Tampilkan Buku Terurut Berdasarkan Judul\n0. Keluar\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan); getchar();

        switch (pilihan) {
            case 1: inputBuku(&root); break;
            case 2: inputPengguna(); break;
            case 3: inorderTraversal(root); break;
            case 4: {
                int id;
                printf("Masukkan ID Pengguna: "); scanf("%d", &id);
                Pengguna *u = findUser(id);
                if (u) printf("Pengguna: %s\n", u->nama);
                else printf("Tidak ditemukan.\n");
                break;
            }
            case 5: pinjamBuku(root); break;
            case 6: kembalikanBuku(root); break;
            case 7: undoTerakhir(root); break;
            case 8: {
                int id;
                printf("Masukkan ID Buku: "); scanf("%d", &id);
                AVLNode *b = findBook(root, id);
                if (!b) printf("Buku tidak ditemukan.\n");
                else {
                    printf("Antrean peminjam untuk \"%s\":\n", b->data.judul);
                    QueueNode *cur = b->antrean.front;
                    if (!cur) printf("(Kosong)\n");
                    while (cur) {
                        Pengguna *u = findUser(cur->idUser);
                        if (u) printf("- ID: %d | Nama: %s\n", u->id, u->nama);
                        else printf("- ID: %d | (Tidak ditemukan)\n", cur->idUser);
                        cur = cur->next;
                    }
                }
                break;
            }
            case 9: tampilkanBukuTerurut(root); break;
            case 0:
                printf("Keluar...\n");
                freeAVL(root);
                break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 0);
    return 0;
}