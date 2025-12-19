#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>

using namespace std;

const char* DISK_NAME = "vdisk.dat";
const int BLOCK_SIZE = 1024;
const int TOTAL_BLOCKS = 50;
const int MAX_FILES = 10;
const int MAGIC_NUMBER = 0xF5DE;
const int RESERVED_BLOCKS = 5;

struct Superblock { int magic; int total_blocks; int free_blocks; bool is_dirty; };
struct Inode { char name[32]; int size; int block_index; bool is_used; };

class SimpleFileSystem {
private:
    fstream disk;
    Superblock sb;
    Inode inode_table[MAX_FILES];

    void format() {
        sb.magic = MAGIC_NUMBER; sb.total_blocks = TOTAL_BLOCKS; sb.free_blocks = TOTAL_BLOCKS - RESERVED_BLOCKS; sb.is_dirty = false;
        for(int i=0; i<MAX_FILES; i++) { inode_table[i].is_used = false; inode_table[i].block_index = -1; }
        disk.open(DISK_NAME, ios::out | ios::binary);
        saveMetadata();
        char zero[BLOCK_SIZE] = {0};
        for(int i=0; i < TOTAL_BLOCKS; i++) disk.write(zero, BLOCK_SIZE);
        disk.close();
    }

    void saveMetadata() {
        disk.seekp(0, ios::beg);
        disk.write((char*)&sb, sizeof(Superblock));
        disk.write((char*)inode_table, sizeof(inode_table));
    }

    void recover() {
        cout << "WARNING:Unclean_Shutdown_Detected;"; 
        int actual_used_count = 0;
        for(int i=0; i<MAX_FILES; i++) if(inode_table[i].is_used) actual_used_count++;
        int correct_free = (TOTAL_BLOCKS - RESERVED_BLOCKS) - actual_used_count;
        if (sb.free_blocks != correct_free) { sb.free_blocks = correct_free; cout << "FIX:Repaired_Free_Block_Count;"; }
        sb.is_dirty = false;
        saveMetadata();
    }

public:
    void mount() {
        disk.open(DISK_NAME, ios::in | ios::out | ios::binary);
        if (!disk.is_open()) { format(); disk.open(DISK_NAME, ios::in | ios::out | ios::binary); }
        disk.seekg(0, ios::beg);
        disk.read((char*)&sb, sizeof(Superblock));
        disk.read((char*)inode_table, sizeof(inode_table));
        if (sb.is_dirty) recover();
    }

    void create(string name, string content) {
        sb.is_dirty = true; saveMetadata();
        for(int i=0; i<MAX_FILES; i++) if(inode_table[i].is_used && inode_table[i].name == name) { cout << "ERROR:File_Exists"; return; }
        int freeInode = -1;
        for(int i=0; i<MAX_FILES; i++) if (!inode_table[i].is_used) { freeInode = i; break; }
        if (freeInode == -1) { cout << "ERROR:Disk_Full"; return; }
        
        bool used_map[TOTAL_BLOCKS] = {false};
        for(int i=0; i<MAX_FILES; i++) if(inode_table[i].is_used) used_map[inode_table[i].block_index] = true;
        int targetBlock = -1;
        for(int i=RESERVED_BLOCKS; i<TOTAL_BLOCKS; i++) if(!used_map[i]) { targetBlock = i; break; }
        if (targetBlock == -1) { cout << "ERROR:No_Blocks_Left"; return; }

        if (content.length() >= BLOCK_SIZE) { cout << "ERROR:Content_Too_Large_For_Block"; return; }

        disk.seekp(targetBlock * BLOCK_SIZE, ios::beg);
        char buffer[BLOCK_SIZE] = {0};
        strncpy(buffer, content.c_str(), BLOCK_SIZE-1);
        disk.write(buffer, BLOCK_SIZE);
        strcpy(inode_table[freeInode].name, name.c_str());
        inode_table[freeInode].size = content.length();
        inode_table[freeInode].block_index = targetBlock;
        inode_table[freeInode].is_used = true;
        sb.free_blocks--;
        sb.is_dirty = false; saveMetadata();
        cout << "SUCCESS:Created_at_Block_" << targetBlock;
    }

    // --- NEW UPDATE FUNCTION ---
    void update(string name, string content) {
        sb.is_dirty = true; saveMetadata();
        for(int i=0; i<MAX_FILES; i++) {
            if(inode_table[i].is_used && inode_table[i].name == name) {
                // Check size limit
                if (content.length() >= BLOCK_SIZE) { cout << "ERROR:Content_Exceeds_Block_Size"; return; }

                int block = inode_table[i].block_index;
                disk.seekp(block * BLOCK_SIZE, ios::beg);
                char buffer[BLOCK_SIZE] = {0};
                strncpy(buffer, content.c_str(), BLOCK_SIZE-1);
                disk.write(buffer, BLOCK_SIZE);
                
                inode_table[i].size = content.length();
                sb.is_dirty = false; saveMetadata();
                cout << "SUCCESS:Updated_Content";
                return;
            }
        }
        cout << "ERROR:File_Not_Found";
    }

    void read(string name) {
        for(int i=0; i<MAX_FILES; i++) {
            if(inode_table[i].is_used && inode_table[i].name == name) {
                disk.seekg(inode_table[i].block_index * BLOCK_SIZE, ios::beg);
                char buffer[BLOCK_SIZE] = {0};
                disk.read(buffer, BLOCK_SIZE);
                cout << buffer; return;
            }
        }
        cout << "ERROR:File_Not_Found";
    }

    void del(string name) {
        sb.is_dirty = true; saveMetadata();
        for(int i=0; i<MAX_FILES; i++) {
            if(inode_table[i].is_used && inode_table[i].name == name) {
                inode_table[i].is_used = false;
                sb.free_blocks++;
                sb.is_dirty = false; saveMetadata();
                cout << "SUCCESS:Deleted"; return;
            }
        }
        cout << "ERROR:File_Not_Found";
    }

    void crash() { sb.is_dirty = true; saveMetadata(); cout << "SUCCESS:System_Halted"; exit(0); }

    void optimize() {
        sb.is_dirty = true; saveMetadata();
        struct FileBuffer { char name[32]; char content[BLOCK_SIZE]; int size; };
        vector<FileBuffer> backups;
        for(int i=0; i<MAX_FILES; i++) if(inode_table[i].is_used) {
            FileBuffer fb; strcpy(fb.name, inode_table[i].name); fb.size = inode_table[i].size;
            disk.seekg(inode_table[i].block_index * BLOCK_SIZE, ios::beg);
            disk.read(fb.content, BLOCK_SIZE);
            backups.push_back(fb);
        }
        for(int i=0; i<MAX_FILES; i++) inode_table[i].is_used = false;
        int currentBlock = RESERVED_BLOCKS;
        for(int i=0; i<backups.size(); i++) {
            disk.seekp(currentBlock * BLOCK_SIZE, ios::beg);
            disk.write(backups[i].content, BLOCK_SIZE);
            inode_table[i].is_used = true;
            strcpy(inode_table[i].name, backups[i].name);
            inode_table[i].size = backups[i].size;
            inode_table[i].block_index = currentBlock;
            currentBlock++;
        }
        sb.is_dirty = false; saveMetadata();
        cout << "SUCCESS:Defragmentation_Complete";
    }

    void list() {
        bool empty = true;
        for(int i=0; i<MAX_FILES; i++) {
            if (inode_table[i].is_used) {
                cout << inode_table[i].name << "," << inode_table[i].block_index << "," << inode_table[i].size << ";";
                empty = false;
            }
        }
        if (empty) cout << "NONE";
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) return 0;
    SimpleFileSystem fs; fs.mount();
    string command = argv[1];
    if (command == "create" && argc >= 4) fs.create(argv[2], argv[3]);
    else if (command == "read" && argc >= 3) fs.read(argv[2]);
    else if (command == "update" && argc >= 4) fs.update(argv[2], argv[3]); // Added Update command
    else if (command == "list") fs.list();
    else if (command == "delete" && argc >= 3) fs.del(argv[2]);
    else if (command == "crash") fs.crash();
    else if (command == "optimize") fs.optimize();
    return 0;
}