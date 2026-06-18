#include "e1000.h"
#include "ports.h"
#include "vga.h"
#include "mm.h"
#include "string.h"

volatile unsigned int* e1000_mem;

#define E1000_REG_CTRL 0x0000
#define E1000_REG_STATUS 0x0008
#define E1000_REG_RCTL 0x0100
#define E1000_REG_TCTL 0x0400
#define E1000_REG_RDBAL 0x2800
#define E1000_REG_RDBAH 0x2804
#define E1000_REG_RDLEN 0x2808
#define E1000_REG_RDH 0x2810
#define E1000_REG_RDT 0x2818
#define E1000_REG_TDBAL 0x3800
#define E1000_REG_TDBAH 0x3804
#define E1000_REG_TDLEN 0x3808
#define E1000_REG_TDH 0x3810
#define E1000_REG_TDT 0x3818

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct e1000_rx_desc {
    unsigned long long addr;
    unsigned short length;
    unsigned short checksum;
    unsigned char status;
    unsigned char errors;
    unsigned short special;
} __attribute__((packed));

struct e1000_tx_desc {
    unsigned long long addr;
    unsigned short length;
    unsigned char cso;
    unsigned char cmd;
    unsigned char status;
    unsigned char css;
    unsigned short special;
} __attribute__((packed));

static struct e1000_rx_desc rx_ring[E1000_NUM_RX_DESC] __attribute__((aligned(16)));
static struct e1000_tx_desc tx_ring[E1000_NUM_TX_DESC] __attribute__((aligned(16)));
static unsigned char rx_buffers[E1000_NUM_RX_DESC][2048] __attribute__((aligned(16)));

static int tx_cur = 0;
static int rx_cur = 0;

void e1000_write_reg(unsigned short offset, unsigned int val) {
    e1000_mem[offset >> 2] = val;
}

unsigned int e1000_read_reg(unsigned short offset) {
    return e1000_mem[offset >> 2];
}

void e1000_setup_rx() {
    // 1. RX ring setup
    for(int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_ring[i].addr = (unsigned long long)(unsigned int)rx_buffers[i];
        rx_ring[i].status = 0;
    }

    e1000_write_reg(E1000_REG_RDBAL, (unsigned int)&rx_ring);
    e1000_write_reg(E1000_REG_RDBAH, 0);
    e1000_write_reg(E1000_REG_RDLEN, E1000_NUM_RX_DESC * 16);
    e1000_write_reg(E1000_REG_RDH, 0);
    e1000_write_reg(E1000_REG_RDT, E1000_NUM_RX_DESC - 1);

    // 2. RCTL: Enable, Broadcast, No Loopback, 2048 byte
    e1000_write_reg(E1000_REG_RCTL, 0x00008002 | (1 << 15) | (1 << 4) | (1 << 3));

    rx_cur = 0;
    vga_print_string("E1000: RX Ring Ready\n", VGA_COLOR_CYAN);
}

void e1000_setup_tx() {
    e1000_write_reg(E1000_REG_TDBAL, (unsigned int)&tx_ring);
    e1000_write_reg(E1000_REG_TDBAH, 0);
    e1000_write_reg(E1000_REG_TDLEN, E1000_NUM_TX_DESC * 16);
    e1000_write_reg(E1000_REG_TDH, 0);
    e1000_write_reg(E1000_REG_TDT, 0);

    // TCTL: Enable, Pad Short Packets, 15 Retries
    e1000_write_reg(E1000_REG_TCTL, 0x000400FA);

    tx_cur = 0;
    vga_print_string("E1000: TX Ring Ready\n", VGA_COLOR_CYAN);
}

void e1000_send_packet(unsigned char* data, int len) {
    // Wait if descriptor not free
    while(!(tx_ring[tx_cur].status & 0x1)) {
        __asm__ __volatile__("pause");
    }

    tx_ring[tx_cur].addr = (unsigned long long)(unsigned int)data;
    tx_ring[tx_cur].length = len;
    tx_ring[tx_cur].cmd = 0x0B; // EOP | IFCS | RS
    tx_ring[tx_cur].status = 0;

    int old_cur = tx_cur;
    tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;

    e1000_write_reg(E1000_REG_TDT, tx_cur);

    vga_print_string("E1000: Packet Sent!\n", VGA_COLOR_LIGHT_GREEN);
}

int e1000_receive_packet(unsigned char* buffer, int max_len) {
    if(rx_ring[rx_cur].status & 0x1) {
        int len = rx_ring[rx_cur].length;
        if(len > max_len) len = max_len;

        memcpy(buffer, rx_buffers[rx_cur], len);
        rx_ring[rx_cur].status = 0;

        int old_cur = rx_cur;
        rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
        e1000_write_reg(E1000_REG_RDT, old_cur);

        return len;
    }
    return 0;
}

void e1000_init(unsigned int bar0) {
    e1000_mem = (volatile unsigned int*)bar0;
    vga_print_string("E1000: Initializing...\n", VGA_COLOR_YELLOW);

    // 1. Reset device - FIXED
    e1000_write_reg(E1000_REG_CTRL, 0x04000000);
    for(volatile int i = 0; i < 1000000; i++); // Bigger delay
    while(e1000_read_reg(E1000_REG_CTRL) & 0x04000000); // Wait for reset done

    vga_print_string("E1000: Reset done\n", VGA_COLOR_GREEN);

    // 2. Clear multicast table
    for(int i = 0; i < 128; i++) {
        e1000_write_reg(0x5200 + (i * 4), 0);
    }

    // 3. Setup RX + TX
    e1000_setup_rx();
    e1000_setup_tx();

    // 4. Enable device
    e1000_write_reg(E1000_REG_CTRL, e1000_read_reg(E1000_REG_CTRL) | (1 << 6));

    vga_print_string("E1000: Ready! Link Up\n", VGA_COLOR_LIGHT_GREEN);
}