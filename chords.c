/*
 * ================================================================
 *  File: chords_driver.c
 *  Author: Tony0M
 *  Date: May 11, 2025
 *  Description: A Linux kernel module for generating musical chords
 *               based on given parameters.
 *
 *  License: GPLv2
 *  Copyright (c) 2025 by TonyOM. All rights reserved.
 * ================================================================
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

static int major;
static char *notes[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const int D_ROOT = 0;
static const int D_2nd = 1;
static const int D_3rd = 2;
static const int D_4th = 3;
static const int D_5th = 4;
static const int D_6th = 5;
static const int D_7th = 6;
static const int D_Oct = 7;
static int formula[7] = {2, 2, 1, 2, 2, 2, 1};
static int formulaSize = 7;
static int noteIndex = 0;
static char note[3];
// static int isItAFlat = 0;
char kernel_buffer[32] = {0};

// Used to return the index of a music note in this system
static int getScaleOffset(const char *musicScale, int l)
{
    for (int i = 0; i < 12; i++)
    {
        if (strncmp(musicScale, notes[i], l) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Sets formula for easier calculation of chords
static void setFormula(char mode)
{
    switch (mode)
    {
    case 'm':
        // Natural Minor Chord
        formula[0] = formula[2] = formula[3] = formula[5] = formula[6] = 2;
        formula[1] = formula[4] = 1;
        formulaSize = 7;
        printk("CHORDS: Is a Minor Chord\n");
        break;
    case '+':
        // Augmented Chord
        formula[0] = formula[2] = formula[4] = 3;
        formula[1] = formula[3] = formula[5] = 1;
        formulaSize = 6;
        printk("CHORDS: Is an Augmented Chord\n");
        break;
    case 'O':
    case '0':
        // Diminished Chord
        formula[0] = formula[2] = formula[4] = formula[6] = 2;
        formula[1] = formula[3] = formula[5] = formula[7] = 1;
        formulaSize = 8;
        printk("CHORDS: Is a Diminished Chord\n");
        break;
    default:
        // Assume Major Chord
        formula[0] = formula[1] = formula[3] = formula[4] = formula[5] = 2;
        formula[2] = formula[6] = 1;
        formulaSize = 7;
        printk("CHORDS: Is a Major Chord\n");
    }
}

// Get note from offset and position. Shift if need be..
static char *getNoteAndShift(int offset, int pos, int shift)
{
    int steps = 0;
    for (int i = 0; i < pos; i++)
    {
        steps += formula[i % formulaSize];
    }
    int val = (offset + steps + shift) % 12;
    return notes[val];
}
// Get note from offset and position
static char *getNote(int offset, int pos)
{
    return getNoteAndShift(offset, pos, 0);
}


// Helper class because I kept getting a segmenation fault
static int isDigit(char c) {
    if (c >= '0' && c <= '9'){
        switch(c){
            case '1':
                return 1;
            case '2':
                return 2;
            case '3':
                return 3;
            case '4': 
                return 4;
            case '5':
                return 5;
            case '6':
                return 6;
            case '7':
                return 7;
            case '8':
                return 8;
            case '9':
                return 9;
            default:
                return 0;
        }
    }
    return -1;
}

// Read function for the driver
static ssize_t my_read(struct file *f, char __user *u, size_t l, loff_t *o)
{
    printk("CHORDS: Reading for Chord %s...\n", kernel_buffer);
    int tracker = strlen(note);

    // Calculate scale/mode
    char mode = kernel_buffer[tracker];
    if (strlen(kernel_buffer) > 1)
    {
            setFormula(kernel_buffer[tracker]);
            tracker++;
    }

    char result[32] = {0};
    char root[3] = {0};
    char third[3] = {0};
    char fifth[3] = {0};

    // Triad Chords (root, third, fifth)
    strcpy(root, getNote(noteIndex, D_ROOT));
    printk("CHORDS: Root note is %s", root);
    strcpy(third, getNote(noteIndex, D_3rd));
    strcpy(fifth, getNote(noteIndex, D_5th));

    // Calculate Seventh key
    int has7th = kernel_buffer[1] == '7' || kernel_buffer[2] == '7';
    char seventh[3] = {0};
    if (has7th)
    {
        tracker++;
        if (kernel_buffer[1] == '7')
        { // dominant 7th chord
            strcpy(seventh, getNoteAndShift(noteIndex, D_7th, -1));
            printk("CHORDS: Chord has Dominant 7th note %s", seventh);
        }
        else if (kernel_buffer[1] == '+')
        { // augmented 7th chord
            strcpy(seventh, getNoteAndShift(noteIndex, D_7th, -2));
            printk("CHORDS: Chord has Augmented 7th note %s", seventh);
        }
        else if (kernel_buffer[1] == '0')
        { // half-diminished 7th
            strcpy(seventh, getNoteAndShift(noteIndex, D_7th, 1));
            printk("CHORDS: Chord has Half-Diminished 7th note %s", seventh);
        }
        else
        { // major/minor/diminished 7th
            strcpy(seventh, getNote(noteIndex, D_7th));
            strcpy(seventh, getNoteAndShift(noteIndex, D_7th, 1));
            switch(mode){
                case 'm':
                    printk("CHORDS: Chord has Minor 7th note %s", seventh);
                break;
                case 'O':
                    printk("CHORDS: Chord has Diminished 7th note %s", seventh);
                break;
                default:
                    printk("CHORDS: Chord has Major 7th note %s", seventh);
                break;
            }
        }
    }

    // Calculate if has sus2 or sus4
    int hasSus2 = 0;
    int hasSus4 = 0;
    int limit = 0;
    while(limit < 2){
        if(kernel_buffer[tracker] == 'S'){
            tracker++;
            if(kernel_buffer[tracker] == '2'){
                if(hasSus2){
                    printk("CHORDS: Extra Sus2 provided. :C\n");
                    return -EINVAL;
                }
                strcpy(third, getNote(noteIndex, 1)); // 2 - 1
                hasSus2 = 1;
                tracker++;
            }
            else if(kernel_buffer[tracker] == '4'){
                if(hasSus4){
                    printk("CHORDS: Extra Sus4 provided. :C\n");
                    return -EINVAL;
                }
                strcpy(fifth, getNote(noteIndex, 3)); // 4 - 1
                hasSus4 = 1;
                tracker++;
            }
            else 
            {
                printk("CHORDS: Invalid format for suspended chord. :C\n");
                return -EINVAL;
            }
        }
        limit++;
    }

    if(hasSus2){
        printk("CHORDS: Chord has a 2nd suspended note %s", third);
    }
    else {
        switch(mode){
            case 'm':
            case 'O':
            case '0':
                printk("CHORDS: Chord has a Minor 3rd note %s", third);
            break;
            default:
                printk("CHORDS: Chord has a Major 3rd note %s", third);
            break;
        }
    }
    if(hasSus4){
        printk("CHORDS: Chord has a 4th Suspended note %s", fifth);
    }
    else {
        switch(mode){
            case '+':
                printk("CHORDS: Chord has an Augmented 5th note %s", fifth);
            break;
            case 'O':
                printk("CHORDS: Chord has a Diminished 5th note %s", fifth);
            break;
            case '0':
            break;
            default:
                printk("CHORDS: Chord has a Perfect 5th note %s", fifth);
            break;
        }
    }

    // Calculate if has add
    int hasAdd = kernel_buffer[tracker] == 'A';
    char add[3] = {0};
    limit = 0;
    int pos = 0;
    int t = 0;
    if (hasAdd)
    {
        tracker++;
        while(limit < 2){
            t = isDigit(kernel_buffer[tracker]);
            if(t >= 0){
                pos *= 10;
                pos += t;
                tracker++;
            }
            limit++;
        }
        if(pos == 0){
            printk("CHORDS: Invalid format for add/extended chord. :C\n");
            return -EINVAL;
        }
        strcpy(add, getNote(noteIndex, pos - 1));
        printk("CHORDS: Chord has an Added %d note %s", pos, fifth);
    }

    // Calculate if has slash
    int hasSlash = kernel_buffer[tracker] == '/';
    char slash[3] = {0};
    if (hasSlash){
        tracker++;
        slash[0] = kernel_buffer[tracker];
        tracker++;
        if(tracker < 16){
            slash[1] = kernel_buffer[tracker];
        }
        tracker++;
        if(slash[0] == '\0'){
            printk("CHORDS: Invalid format for a slash (inversion) chord. :C\n");
            return -EINVAL; 
        }
        printk("CHORDS: Chord has a Slash (Inversion) note %s", slash);
    }

    // Put it all together now! Print out the chord spelling
    if(hasSlash){
        strcpy(result, slash);
        strcat(result, "(bass)-");
        strcat(result, root);
    }
    else{
        strcpy(result, root);
    }
    
    if(!hasSlash || strcmp(third, slash) != 0){
        strcat(result, "-");
        strcat(result, third);
    }

    if(!hasSlash || strcmp(fifth, slash) != 0)
    {
        strcat(result, "-");
        strcat(result, fifth);
    }
    
    if(has7th){
        strcat(result, "-");
        strcat(result, seventh);
    }
    if(hasAdd){
        strcat(result, "-");
        strcat(result, add);
    }

    printk("CHORDS: %s\n", result);

    result[strlen(result)] = '\0';

    // return to user buffer
    copy_to_user(u, result, strlen(result));

    return strlen(result);
}

// Write function for the driver
static ssize_t my_write(struct file *f, const char __user *u, size_t l, loff_t *o)
{
    printk("CHORDS: Writing...\n");
    printk("CHORDS: Input chord name and receive notes in that chord.\n");

    if (!u)
    {
        return -EINVAL;
    }
    if (!access_ok(u, l))
    {
        return -EFAULT;
    }
    if (l == 0)
    {
        return 0;
    }

    memset(kernel_buffer, 0, 16 * sizeof(char));

    copy_from_user(kernel_buffer, u, min(l, sizeof(kernel_buffer)));

    // Extract note
    note[0] = kernel_buffer[0];
    note[1] = '\0';
    note[2] = '\0';
    int copy_size = 1;
    if (kernel_buffer[1] == '#' || kernel_buffer[1] == 'b')
    {
        note[1] = kernel_buffer[1];
    }
    strncpy(note, kernel_buffer, copy_size);
    note[copy_size] = '\0'; // Ensure null termination

    noteIndex = getScaleOffset(note, copy_size);
    if (noteIndex < 0)
    {
        printk("CHORDS: Note %s does not exist in this driver :C\n", note);
        note[0] = 'C';
        note[1] = '\0'; // Ensure null termination
        return -EFAULT;
    }

    printk("CHORDS: Chord %s written! Call read to receive chord response.", kernel_buffer);

    return 0;
}


// fops
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = my_write,
    .read = my_read};

// init
static int __init my_init(void)
{
    major = register_chrdev(0, "hello_cdev", &fops);
    if (major < 0)
    {
        pr_err("hello_cdev - Error registering chrdev\n");
        return major;
    }
    printk("CHORDS: Hello! (Major Device Number: %d)\n", major);
    return 0;
}

//exit
static void __exit my_exit(void)
{
    unregister_chrdev(major, "chords");
    printk("CHORDS: Goodbye!");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TonyOM");
MODULE_DESCRIPTION("A simple driver for generating a musical chord based on given parameters.");