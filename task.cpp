#include <string.h>
#include <iostream>
#include <stdlib.h>
#include "os_file.h"

using namespace std;

struct node {
    node* parent;
    char* name;
    int size, is_dir, children_len = 1;
    node* children = (node *) malloc(sizeof(node) * children_len);
};

node *current, *root;
int free_space, created = 0;

// Служебные функции
void push_child(node* &array, int &len, node &item){ //длина массива всегда на 1 больше чем количество элементов
    array[len - 1] = item; //кладем в конец массива
    array = (node*)realloc(array, sizeof(node) * (len++));
}

void remove_dir(node* rmv_dir) {
    if (rmv_dir->is_dir) {
        for (int i = 0; i < rmv_dir->children_len; i++) {
            remove_dir(&rmv_dir->children[i]);
        }
    }
    delete rmv_dir;
}

int valid_name(char* name) { //Проверка на правильность названия создаваемой директории или файла
    if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0 || !strlen(name) || strlen(name) > 32)
        return 0;
    for (int i = 0; i < strlen(name); i++) {
        if (name[i] != '_' && name[i] != '.' && !(name[i] >= 'a' && name[i] <= 'z') && !(name[i] >= '0' && name[i] <= '9'))
            return 0;
    }
    return 1;
}

node* find_host_address(char* route) { //Поиск папки по пути
    if (!strlen(route)) return nullptr; // если путь пустой

    node *target = (route[0] == '/') ? root : current; //если начинается на "/" значит ищем с самого начала, если нет то от текущего

    char *dir = strtok(route, "/");

    while (dir != nullptr) {
        if (strcmp(dir, ".") != 0) {
            if (strcmp(dir, "..") == 0) {
                if (target->parent)
                    target = target->parent;
                else
                    return nullptr;
            } else { // абс путь
                for(int i = 0; i < target->children_len; i++){
                    if(strcmp((&target->children[i])->name, dir) == 0){
                        target = &target->children[i];
                    }
                }
            }
        }

    }

    return current;
}

// Основные функции
int my_create(int disk_size) {
    if (disk_size <= 0 && created) return 0;

    created = 1;
    free_space = disk_size;

    root = new node();
    root->is_dir = 1;
    root->size = 0;

    current = root;
    return 1;
}

int my_destroy() {
    if (!created) return 0;
    created = 0;

    remove_dir(root);
    root = nullptr;
    current = nullptr;
    return 1;
}

int my_create_dir(const char* path_to_dir) {
    if (!created) return 0;

    char *tmp = "/";
    if(strrchr(path_to_dir, '/') != nullptr) {
        tmp = strchr(path_to_dir, '/'); //в tmp сейчас лежит название папки вместе с / "/dir1"
    } else {
        strcat(tmp, path_to_dir);
    }

    char dirName[strlen(tmp)-1];

    int j, i;
    for(j = 0; j < strlen(tmp) - 1; j++){
        dirName[j] = tmp[j + 1];
    }
    dirName[j + 1] = '\0'; // теперь в dirName лежит название папки без / "dir1"

    char folder[strlen(path_to_dir) - strlen(dirName)];

    for(i = 0; i < strlen(path_to_dir) - strlen(dirName); i++){
        folder[i] = path_to_dir[i];
    }
    folder[i + 1] = '\0'; //в folder лежит название родительской папки

    if(!valid_name(dirName) || find_host_address(folder) == nullptr) return 0; // проверка на существование и корректности названия

    node *parent = find_host_address(folder);
    node *new_node = new node(); //создаем структуру для нового узла
    new_node->name = dirName;
    new_node->is_dir = 1;
    new_node->size = 0;
    new_node->parent = parent;
    push_child(parent->children, parent->children_len, *new_node); // у parent появился потомок new_node
    return 1;
}

int my_create_file(const char* path_to_file, int file_size) {
    if (!created && free_space < file_size) return 0;
    char* tmp = strrchr(path_to_file, '/');
    char dirName[strlen(tmp) - 1];

    int j, i;
    for(j = 0; j < strlen(tmp) - 1; j++){
        dirName[j] = tmp[j + 1];
    }
    dirName[j + 1] = '\0';

    char folder[strlen(path_to_file) - strlen(dirName)];

    for(i = 0; i < strlen(path_to_file) - strlen(dirName); i++){
        folder[i] = path_to_file[i];
    }
    folder[i + 1] = '\0';

    if(!valid_name(dirName) || find_host_address(folder) == nullptr) return 0;

    node *parent = find_host_address(folder);
    node *new_node = new node(); //создаем структуру для нового узла
    new_node->name = dirName;
    new_node->is_dir = 0;
    new_node->size = file_size;
    new_node->parent = parent;
    push_child(parent->children, parent->children_len, *new_node); // у parent появился потомок new_node
    return 1;
 }

int my_change_dir(const char* path_to_dir) {
    if (!created) return 0;

    char path = *path_to_dir;
    node* c_node = find_host_address(&path);

    if (c_node && c_node->is_dir) {
        current = c_node;
    }
    return 1;
}

void my_get_cur_dir(char* dst) {
    char* path = "";
    if (current == root) path = "/";

    for (node* c_node = current; c_node && c_node->parent; c_node = c_node->parent) {
        strcat(path, c_node->name);
        strcat(path, "/");
    }

    strcpy(dst, path);
 }

int my_move(const char* old_path, const char* new_path) {
    char old_p = *old_path;
    char new_p = *new_path;

    node *old_dir = find_host_address(&old_p);
    node *new_dir = find_host_address(&new_p);

    if ((old_dir == nullptr || new_dir == nullptr) || (old_dir == new_dir)) return 0;

    if(old_dir->name == current->name){
        current = root;
    }

    node* new_node = new node();
    new_node->name = old_dir->name;
    new_node->is_dir = old_dir->is_dir;
    new_node->size = old_dir->size;
    new_node->parent = new_dir;
    push_child(new_dir->children, new_dir->children_len, *new_node); // у parent появился потомок new_node

    remove_dir(old_dir);

    return 1;
}

void setup_file_manager(file_manager_t* fm) {
    fm->create = my_create;
    fm->destroy = my_destroy;
    fm->create_dir = my_create_dir;
    fm->create_file = my_create_file;
    fm->change_dir = my_change_dir;
    fm->get_cur_dir = my_get_cur_dir;
    fm->move = my_move;
}