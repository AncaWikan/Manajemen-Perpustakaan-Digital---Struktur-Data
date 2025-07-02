#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX_TITLE 100
#define MAX_NAME 50
#define MAX_PEMINJAMAN 100
#define MAX_STACK 100
#define HASH_SIZE 101

/* ------------------- ADT Buku ------------------- */
typedef struct Buku {
    int id;
    char judul[MAX_TITLE];
    char penulis[MAX_NAME];
    int tahun;
    char kategori[50];
    bool tersedia;
    struct Buku *next; // untuk linked list
} Buku;

/* ------------------- ADT Pengguna ------------------- */
typedef struct Pengguna {
    int id;
    char nama[MAX_NAME];
} Pengguna;

/* ------------------- ADT Peminjaman ------------------- */
typedef struct Peminjaman {
    int idUser;
    int idBuku;
    char aksi[10]; // "pinjam" atau "kembali"
} Peminjaman;

/* ------------------- Stack untuk Undo ------------------- */
Peminjaman undoStack[MAX_STACK];
int top = -1;

void pushUndo(Peminjaman p) {
    if (top < MAX_STACK - 1) {
        undoStack[++top] = p;
    }
}

Peminjaman popUndo() {
    if (top >= 0) {
        return undoStack[top--];
    }
    Peminjaman kosong = {-1, -1, ""};
    return kosong;
}

/* ------------------- Queue untuk antrean peminjam ------------------- */
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
    if (!newNode) {
        printf("Gagal alokasi memori.\n");
        return;
    }
    newNode->idUser = idUser;
    newNode->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

int dequeue(Queue *q) {
    if (q->front == NULL) return -1;
    QueueNode *temp = q->front;
    int id = temp->idUser;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    return id;
}

void clearQueue(Queue *q) {
    while (q->front != NULL) {
        QueueNode *temp = q->front;
        q->front = q->front->next;
        free(temp);
    }
    q->rear = NULL;
}

/* ------------------- AVL Tree ------------------- */
typedef struct AVLNode {
    Buku data;
    struct AVLNode *left, *right;
    int height;
    Queue antrean;
} AVLNode;

int height(AVLNode *node) {
    if (node == NULL) return 0;
    return node->height;
}

int getBalance(AVLNode *node) {
    if (node == NULL) return 0;
    return height(node->left) - height(node->right);
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

AVLNode* newAVLNode(Buku data) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
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
    if (node == NULL)
        return newAVLNode(data);

    if (data.id < node->data.id)
        node->left = insertAVL(node->left, data);
    else if (data.id > node->data.id)
        node->right = insertAVL(node->right, data);
    else
        return node;

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = getBalance(node);

    if (balance > 1 && data.id < node->left->data.id)
        return rightRotate(node);
    if (balance < -1 && data.id > node->right->data.id)
        return leftRotate(node);
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
    if (root == NULL || root->data.id == id)
        return root;
    if (id < root->data.id)
        return findBook(root->left, id);
    else
        return findBook(root->right, id);
}

void inorderTraversal(AVLNode *root) {
    if (root != NULL) {
        inorderTraversal(root->left);
        printf("ID: %d | Judul: %s | Penulis: %s | Tersedia: %s\n",
               root->data.id, root->data.judul, root->data.penulis,
               root->data.tersedia ? "Ya" : "Tidak");
        inorderTraversal(root->right);
    }
}

void freeAVL(AVLNode *root) {
    if (root != NULL) {
        freeAVL(root->left);
        freeAVL(root->right);
        clearQueue(&root->antrean);
        free(root);
    }
}

/* ------------------- Hash Table Pengguna ------------------- */
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
    UserHashNode* newNode = (UserHashNode*)malloc(sizeof(UserHashNode));
    newNode->user = user;
    newNode->next = userTable[index];
    userTable[index] = newNode;
}

Pengguna* findUser(int id) {
    int index = hash(id);
    UserHashNode* current = userTable[index];
    while (current != NULL) {
        if (current->user.id == id)
            return &current->user;
        current = current->next;
    }
    return NULL;
}

/* ------------------- Fungsi Input & Peminjaman ------------------- */
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
        Peminjaman p = {idUser, idBuku, "pinjam"};
        pushUndo(p);
        printf("Peminjaman berhasil.\n");
    } else {
        printf("Buku sedang dipinjam. Ditambahkan ke antrean.\n");
        enqueue(&b->antrean, idUser);
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
        Peminjaman p = {idUser, idBuku, "kembali"};
        pushUndo(p);
        if (b->antrean.front != NULL) {
            int nextUser = dequeue(&b->antrean);
            printf("Buku otomatis dipinjam oleh antrean user ID %d.\n", nextUser);
            b->data.tersedia = false;
            Peminjaman autoPinjam = {nextUser, idBuku, "pinjam"};
            pushUndo(autoPinjam);
        } else {
            printf("Pengembalian berhasil.\n");
        }
    } else {
        printf("Buku sudah tersedia. Tidak perlu dikembalikan.\n");
    }
}

void undoTerakhir(AVLNode *root) {
    Peminjaman p = popUndo();
    if (p.idUser == -1) {
        printf("Tidak ada aksi untuk di-undo.\n");
        return;
    }
    AVLNode *b = findBook(root, p.idBuku);
    if (!b) return;
    if (strcmp(p.aksi, "pinjam") == 0) {
        b->data.tersedia = true;
        printf("Undo: Buku ID %d dikembalikan.\n", p.idBuku);
    } else if (strcmp(p.aksi, "kembali") == 0) {
        b->data.tersedia = false;
        printf("Undo: Buku ID %d dipinjam ulang.\n", p.idBuku);
    }
}

/* ------------------- Main ------------------- */
int main() {
    AVLNode *root = NULL;
    int pilihan;
    do {
        printf("\n===== Menu Perpustakaan =====\n");
        printf("1. Tambah Buku\n2. Tambah Pengguna\n3. Lihat Daftar Buku\n4. Cari Pengguna\n5. Pinjam Buku\n6. Kembalikan Buku\n7. Undo Terakhir\n8. Tampilkan Antrean Buku\n0. Keluar\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan); getchar();

        switch(pilihan) {
            case 1: inputBuku(&root); break;
            case 2: inputPengguna(); break;
            case 3: inorderTraversal(root); break;
            case 4: {
                int cariID;
                printf("Masukkan ID Pengguna: "); scanf("%d", &cariID);
                Pengguna *u = findUser(cariID);
                if (u) printf("Pengguna ditemukan: %s\n", u->nama);
                else printf("Pengguna tidak ditemukan.\n");
                break;
            }
            case 5: pinjamBuku(root); break;
            case 6: kembalikanBuku(root); break;
            case 7: undoTerakhir(root); break;
            case 8: {
                int idBuku;
                printf("Masukkan ID Buku: "); scanf("%d", &idBuku);
                AVLNode *b = findBook(root, idBuku);
                if (!b) printf("Buku tidak ditemukan.\n");
                else {
                    printf("Antrean peminjam untuk buku \"%s\":\n", b->data.judul);
                    QueueNode *current = b->antrean.front;
                    if (!current) printf("(Antrean kosong)\n");
                    while (current != NULL) {
                        Pengguna *u = findUser(current->idUser);
                        if (u) printf("- ID: %d | Nama: %s\n", u->id, u->nama);
                        else printf("- ID: %d | (Pengguna tidak ditemukan)\n", current->idUser);
                        current = current->next;
                    }
                }
                break;
            }
            case 0:
                printf("Keluar dari program.\n");
                freeAVL(root);
                break;
            default: printf("Pilihan tidak valid.\n");
        }
    } while (pilihan != 0);

    return 0;
}
