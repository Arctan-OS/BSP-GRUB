#include <mm/bank.h>

static struct ARC_BankNode bank_nodes[1024];
static int bank_i = 0;

int bank_add_pfreelist(struct ARC_BankMeta *meta, struct ARC_FreelistMeta *allocator) {
        if (meta == NULL || allocator == NULL || meta->type != ARC_MM_BANK_TYPE_PFREELIST) {
                return -1;
        }

        struct ARC_BankNode *node = &bank_nodes[bank_i++];

        node->allocator_meta = allocator;
        node->next = meta->first;
        meta->first = node;

        return 0;
}

int bank_remove_pfreelist(struct ARC_BankMeta *meta, struct ARC_FreelistMeta *allocator) {
        if (meta == NULL || allocator == NULL || meta->type != ARC_MM_BANK_TYPE_PFREELIST) {
                return -1;
        }

        struct ARC_BankNode *node = meta->first;
        struct ARC_BankNode *prev = NULL;
        while (node != NULL && node->allocator_meta != allocator) {
                prev = node;
                node = node->next;
        }

        if (node == NULL) {
                return -2;
        }

        if (prev != NULL) {
                prev->next = node->next;
        } else {
                meta->first = node->next;
        }
        
        // **Free the node**
        // Can't be done here because of the basic bump allocator

        return 0;
}

int init_static_bank(struct ARC_BankMeta *meta, int type) {
        if (meta == NULL) {
                return -1;
        }

        meta->first = NULL;
        meta->type = type;

        return 0;
}