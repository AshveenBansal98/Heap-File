#include <bits/stdc++.h>
#define DISK_FILE_SIZE 1000
#define DISK_PAGE_SIZE 100
#define DIR_ENTRY_LENGTH 13
using namespace std;



struct DirectoryEntry {
    int id;
    int length;
    int start;
    bool valid;
    DirectoryEntry() {
        this->id = 0;
        this->length = 0;
        this->start = 0;
        this->valid = false;
    }
    DirectoryEntry(int id, int length, int start, bool valid) {
        this->id = id;
        this->length = length;
        this->start = start;
        this->valid = valid;
    }
};

class Page{

    public:
        vector<DirectoryEntry> arr;
        int spaceLeft;
        int dirSlotCount;

        //initializes an empty page with initial record entry
        Page(){
            this->arr.resize(1, DirectoryEntry());
            this->spaceLeft = DISK_PAGE_SIZE - (sizeof(this->spaceLeft)+sizeof(this->dirSlotCount)+this->arr.size()*DIR_ENTRY_LENGTH);
            //printf("%d", this->spaceLeft);
            this->dirSlotCount = this->arr.size();
        }
};

struct Node {
    Page data;
    struct Node* next;
    struct Node* prev;
};

class DiskFile{
    public:
        struct Node * nodePointer = NULL;
        int totalPages;

        DiskFile(){ }

        DiskFile(int n, bool * create_diskFile){
            if(n * DISK_PAGE_SIZE > DISK_FILE_SIZE - (sizeof(nodePointer)+sizeof(totalPages))){
                    printf("Number of Pages must not exhaust maximum allowable DiskFile size %d\n", DISK_PAGE_SIZE);
                    *create_diskFile = false;
                    return;
            }
            for(int i=0; i<n; i++)
                appendPages(Page());
            printf("Created %d number of pages of size %d bytes each in DiskFile...\n", n, DISK_PAGE_SIZE);
            printf("Printing created initial DiskFile...\n");
            printPages();
            this->totalPages = n;
            *create_diskFile = true;
        }

        void appendPages(Page new_data);
        void printPages();
        bool insertRecord(int rec_id, int rec_length);
        void deleteRecord(int rec_id);

};

/* Given a reference (pointer to pointer) to the head
of a list and an int, appends a new node at the end  */
void DiskFile :: appendPages(Page new_data){

    Node* new_node = new Node();
    new_node->data  = new_data;
    new_node->next = NULL;
    new_node->prev = NULL;
    if (nodePointer == NULL)
    {
       nodePointer = new_node;
       return;
    }
    Node* last = nodePointer;
    while (last->next != NULL)
        last = last->next;
    last->next = new_node;
    new_node->prev = last;
}

/* Given a reference (pointer to pointer) to the head
of a list and a record details, it inserts the record in the DiskFIle  */
bool DiskFile :: insertRecord(int rec_id, int rec_length){
    if(rec_length > (DISK_PAGE_SIZE - (2*sizeof(int)+DIR_ENTRY_LENGTH))){
        printf("Record length should be less than effective allowable Page size %d, where the maximum Page size is %d\n", (int)(DISK_PAGE_SIZE - (2*sizeof(int)+DIR_ENTRY_LENGTH)), DISK_PAGE_SIZE);
        return false;
    }
    // else if((this->totalPages * DISK_PAGE_SIZE) > (DISK_FILE_SIZE  - (sizeof(this->nodePointer)+sizeof(this->totalPages)))){
    //     printf("Number of Pages must not exhaust effective allowable DiskFile size %d, where the maximum DiskFile size is %d\n", (DISK_FILE_SIZE  - (int)(sizeof(this->nodePointer)+sizeof(this->totalPages))), DISK_FILE_SIZE);
    //     *inserted_records = false;
    //     return;
    // }

    struct Node *last = nodePointer;
    int count = 0;

    while(last != NULL) {
        count = count+1;
        /* Case-1: When DataPages are empty at the beginning */
        if (last->data.arr.size() == 1 && last->data.arr[0].valid == false && last->data.arr[0].length == 0){
            last->data.arr[0].id = rec_id;
            last->data.arr[0].length = rec_length;
            last->data.arr[0].start = 0;
            last->data.arr[0].valid = true;
            last->data.spaceLeft = last->data.spaceLeft - rec_length;
            last->data.dirSlotCount = last->data.arr.size();
            printf("Record inserted in Page : %d \n", count);
            return true;
        }

    /* Case-2: When an empty slot is available from deletion to hold the Record */
        for(int i=0; i<last->data.arr.size(); i++){
            if(last->data.arr[i].valid == false){
                bool space_available = (rec_length <= last->data.arr[i].length);
                if(space_available){ // if last directory is empty, it has the space left following it
                    if (i == last->data.arr.size() - 1){
                        last->data.spaceLeft += (last->data.arr[i].length - rec_length);
                    }
                    last->data.arr[i].id = rec_id;
                    last->data.arr[i].length = rec_length;
                    last->data.arr[i].valid = true;
                    printf("Record inserted in an empty slot of Page : %d \n", count);

                    return true;
                }
            }
        }
    /* Case-3: When no empty slot is available and if existing Page can accommodate the data at the end */
        if (last->data.spaceLeft - (rec_length + DIR_ENTRY_LENGTH) >= 0){
            int next_start_idx = last->data.arr.back().start + last->data.arr.back().length;
            last->data.arr.push_back(DirectoryEntry(rec_id, rec_length, next_start_idx, true));
            last->data.spaceLeft = last->data.spaceLeft - (rec_length + DIR_ENTRY_LENGTH);
            last->data.dirSlotCount = last->data.arr.size();
            printf("Record inserted in Page : %d \n", count);
            return true;
        }

/* Case-4: When no empty slot is available and if existing Page can not accommodate the data at the end,
 then go to next page to check availability */
        if(count<this->totalPages){
            printf("Exhausted space in Page %d. Moving on to Page %d to insert Record...\n", count, count+1);
        }
 /* Case-5: When no empty slot is available and no existing Page can accommodate the data,
 then append a new page if maximum allowable DiskFile size is not exhausted */
        else{
            if((this->totalPages + 1) * DISK_PAGE_SIZE <=  (DISK_FILE_SIZE  - (sizeof(this->nodePointer)+sizeof(this->totalPages)))){
                printf("Exhausted space in Page %d. Page %d is created to insert this new Record!\n", count, count+1);
                appendPages(Page());
                this->totalPages++;
            }
            else{
                printf("Exhausted effective allowable DiskFile size %d, where the maximum DiskFile size is %d\n", (DISK_FILE_SIZE  - (int)(sizeof(this->nodePointer)+sizeof(this->totalPages))), DISK_FILE_SIZE);
                return false;
            }
        }

      last = last->next;
        
    }
}

/* Given a reference pointer to the head
of a list and an Record ID, it deletes the corresponding record in a Page  */
void DiskFile :: deleteRecord(int rec_id){
    struct Node *last = nodePointer;
    bool flag = false; /* To check for duplicate entries. Currently deletes all
    duplicate entries by traversing all the pages. */
    int count = 0;
    while(last != NULL) {
        count++;
        for(int i=0; i<last->data.arr.size(); i++){
            if(last->data.arr[i].id == rec_id && last->data.arr[i].valid == true){
                last->data.arr[i].id = 0;
                int gap = 0;
                if (i != last->data.arr.size()-1){
                    gap = last->data.arr[i+1].start - (last->data.arr[i].start + last->data.arr[i].length);
                }

                /* Claim the left-over space through gap, restores original slot's length but not defined for last Record of a Page */
                last->data.arr[i].length = last->data.arr[i].length + gap;
                last->data.arr[i].valid = false;
                flag = true;
                printf("Record deleted from Page : %d \n", count);
                /* No return after one deletion. After one entry deletion it checks
                for all the duplicate entries matching IDs and deletes them. */
            }
        }
      last = last->next;
    }

  if(flag==false) // If not a single deletion has happened
    printf("Sorry the entered Record ID does not exist in any of the Pages...\n");

  return;
}

/* This function prints contents of DiskFIle starting in terms of Pages & Records */
void DiskFile :: printPages(){
    struct Node* node = nodePointer;
    printf("DiskFile traversal, displaying Pages with Records <ID,Start,Length-Gap,Valid> :\n");
    int count = 0;
    while (node != NULL) {
        count = count+1;
        printf("Page:%d  Records{", count);
        for(int i=0; i<node->data.arr.size(); i++){
            int gap = (i == node->data.arr.size()-1) ? 0 : node->data.arr[i+1].start - (node->data.arr[i].start + node->data.arr[i].length);
            if(i!=node->data.arr.size()-1)
                printf("<%d,%d,%d-%d,%s> ", node->data.arr[i].id, node->data.arr[i].start, node->data.arr[i].length, gap, node->data.arr[i].valid ? "true" : "false");
            else
                printf("<%d,%d,%d-%d,%s>", node->data.arr[i].id, node->data.arr[i].start, node->data.arr[i].length, gap, node->data.arr[i].valid ? "true" : "false");
        }
        printf("}\n");
        node = node->next;
    }
}

int main()
{
    DiskFile d;
    bool create_diskFile = false;
    bool inserted_records = false;
    while(true){
        int x;
        printf("\nEnter :- 1:Create New DiskFile, 2:Insert Record, 3:Delete Record, 4:Display DiskFile Structure, -1:Exit\n");
        printf("Choice : ");
        cin >> x;
        switch (x)
        {
           case 1: {
                       if(create_diskFile==false){
                           printf("Enter number of DataPages to create : ");
                           int n;
                           cin >> n;
                           d = DiskFile(n, &create_diskFile);
                       }
                       else
                           printf("DiskFile has already been created.\n");
                   }
                   break;

           case 2: {
                       if(create_diskFile){
                           printf("Enter Record identifier in int to insert into Pages : ");
                           int id;
                           cin >> id;
                           printf("Enter Record length in bytes : ");
                           int l;
                           cin >> l;
                           bool status = d.insertRecord(id,l);
                           if (!status)
                              cout << "not inserted" << endl;

                       }
                       else
                           printf("You can not enter Records into Pages without creating the initial DiskFile.\n");
                   }
                   break;

           case 3: {
                       if(create_diskFile){
                           printf("Enter Record identifier in int to delete from Pages : ");
                           int id;
                           cin >> id;
                           d.deleteRecord(id);
                       }
                       else
                           printf("You can not delete Records without creating the initial DiskFile or without inserting few Records into it.\n");
                   }
                   break;

           case 4: {
                       if(create_diskFile){
                           d.printPages();
                       }
                       else
                           printf("You can not view Pages & Records without creating the initial DiskFile.\n");
                   }
                   break;

           case -1: exit(0);

           default: printf("Choice other than 1, 2, 3, 4 and -1\n");
                    continue;
        }
    }
}
