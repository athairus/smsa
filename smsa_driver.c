////////////////////////////////////////////////////////////////////////////////
//
//  File           : smsa_driver.c
//  Description    : This is the driver for the SMSA simulator.
//
//   Author        :
//   Last Modified :
//

// Include Files
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

// Project Include Files
#include <smsa_driver.h>
#include <cmpsc311_log.h>

// Defines
#define null 0

// Functional Prototypes
uint32_t smsa_cmdgen( uint8_t op, uint8_t drum, uint8_t block );
uint32_t allones( uint8_t power );
int smsa_save();
int smsa_load();

//
// Global data
FILE *disk_array;

// Interfaces

////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vmount
// Description  : Mount the SMSA disk array virtual address space
//
// Inputs       : none
// Outputs      : -1 if failure or 0 if successful

int smsa_vmount( void ) {

    return smsa_load();
    
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vunmount
// Description  :  Unmount the SMSA disk array virtual address space
//
// Inputs       : none
// Outputs      : -1 if failure or 0 if successful

int smsa_vunmount( void ) {

    return smsa_save();
    
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vread
// Description  : Read from the SMSA virtual address space
//
// Inputs       : addr - the address to read from
//                len - the number of bytes to read
//                buf - the place to put the read bytes
// Outputs      : -1 if failure or 0 if successful

int smsa_vread( SMSA_VIRTUAL_ADDRESS addr, uint32_t len, unsigned char *buf ) {
    uint32_t op = 0;    // smsa operation code
    int ret = 0;        // return value
    
    // sanity check
    if( addr + len > MAX_SMSA_VIRTUAL_ADDRESS ) return -1;
    
    // seek to first drum to read
    uint8_t drum = ( addr >> 16 ) & allones( 4 );
    op = smsa_cmdgen( SMSA_SEEK_DRUM, drum, null );
    ret = smsa_operation( op, buf );
    
    if( ret != 0 ) return -1;
    
    // seek to first block to read
    uint8_t block = ( addr >> 8 ) & allones( 8 ) ;
    op = smsa_cmdgen( SMSA_SEEK_BLOCK, null, block );
    ret = smsa_operation( op, buf );
    
    if( ret != 0 ) return -1;
    
    // loop variable declerations
    uint16_t bytes_modified = 0;                                            // number of bytes touched in each loop, usually SMSA_BLOCK_SIZE
    uint8_t block_data[SMSA_BLOCK_SIZE];                                    // buffer that holds data for a single block
    uint8_t block_start_offset = ( addr & allones( 8 ) );                   // offset from the beginning of a block where writing begins (only applies to very first block in a read/write)
    uint8_t block_end_offset = ( addr + len ) & allones( 8 );               // offset from the beginning of a block where writing ends (only applies to very last block in a read/write)
    int start = 1;                                                          // on the first block of the first drum in the job?
    
    while( len != 0 ) {
    
        logMessage( LOG_INFO_LEVEL, "block=0x%X, len=0x%X", block, len );
        
        // read block
        op = smsa_cmdgen( SMSA_DISK_READ, null, null );
        ret = smsa_operation( op, block_data ); // this moves the block pointer forward one block once it's done
        
        if( ret != 0 ) return -1;
        
        // do something with the read data
        
        uint8_t *foo = block_data;
        
        if( ( start == 1 ) && ( addr >> 8 == ( addr + len ) >> 8 ) ) { // just started and on the first and only drum and block of the job?
            logMessage( LOG_INFO_LEVEL, "case 1 (len<256)" );
            bytes_modified = block_end_offset - block_start_offset;
            foo += block_start_offset;
            start = 0;
        }
        else if( start == 1 ) { // otherwise, just started?
            logMessage( LOG_INFO_LEVEL, "case 2 (begin)" );
            bytes_modified = ( SMSA_BLOCK_SIZE - block_start_offset );
            foo += block_start_offset;
            start = 0;
        }
        else if( len < SMSA_BLOCK_SIZE ) { // otherwise, have less than a block's worth of data to go?
            logMessage( LOG_INFO_LEVEL, "case 3 (end)" );
            bytes_modified = block_end_offset;
        }
        else bytes_modified = SMSA_BLOCK_SIZE;
        
        memcpy( buf, foo, bytes_modified );
        
        buf += bytes_modified;
        len -= bytes_modified;
        
        // move to next drum and seek to block 0 if at the end of the current drum
        if( block == ( SMSA_BLOCK_SIZE - 1 ) ) {
            drum++;
            op = smsa_cmdgen( SMSA_SEEK_DRUM, drum, null );
            ret = smsa_operation( op, buf );
            
            //op = smsa_cmdgen( SMSA_SEEK_BLOCK, null, 0 );
            //ret = smsa_operation( op, buf );
        }
        
        // block will overflow to 0 by design when it hits 255
        // block now matches the actual block pointer, which was moved forward when the read was done
        block++;
        
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vwrite
// Description  : Write to the SMSA virtual address space
//
// Inputs       : addr - the address to write to
//                len - the number of bytes to write
//                buf - the place to read the read from to write
// Outputs      : -1 if failure or 0 if successful

int smsa_vwrite( SMSA_VIRTUAL_ADDRESS addr, uint32_t len, unsigned char *buf ) {

    uint32_t op = 0;    // smsa operation code
    int ret = 0;        // return value
    
    // sanity check
    if( addr + len > MAX_SMSA_VIRTUAL_ADDRESS ) return -1;
    
    // seek to first drum to read
    uint8_t drum = ( addr >> 16 ) & allones( 4 );
    op = smsa_cmdgen( SMSA_SEEK_DRUM, drum, null );
    ret = smsa_operation( op, buf );
    
    if( ret != 0 ) return -1;
    
    // seek to first block to read
    uint8_t block = ( addr >> 8 ) & allones( 8 ) ;
    op = smsa_cmdgen( SMSA_SEEK_BLOCK, null, block );
    ret = smsa_operation( op, buf );
    
    if( ret != 0 ) return -1;
    
    // loop variable declerations
    uint16_t bytes_modified = 0;                                            // number of bytes touched in each loop, usually SMSA_BLOCK_SIZE
    uint8_t block_data[SMSA_BLOCK_SIZE];                                    // buffer that holds data for a single block
    uint8_t block_start_offset = ( addr & allones( 8 ) );                   // offset from the beginning of a block where writing begins (only applies to very first block in a read/write)
    uint8_t block_end_offset = ( addr + len ) & allones( 8 );               // offset from the beginning of a block where writing ends (only applies to very last block in a read/write)
    int start = 1;                                                          // on the first block of the first drum in the job?
    
    while( len != 0 ) {
    
        logMessage( LOG_INFO_LEVEL, "block=0x%X, len=0x%X", block, len );
        
        // read block
        op = smsa_cmdgen( SMSA_DISK_READ, null, null );
        ret = smsa_operation( op, block_data ); // this moves the block pointer forward one block once it's done
        
        if( ret != 0 ) return -1;
        
        // do something with the read data
        
        uint8_t *foo = block_data;
        
        if( ( start == 1 ) && ( addr >> 8 == ( addr + len ) >> 8 ) ) { // just started and on the first and only drum and block of the job?
            logMessage( LOG_INFO_LEVEL, "case 1 (len<256)" );
            bytes_modified = block_end_offset - block_start_offset;
            foo += block_start_offset;
            start = 0;
        }
        else if( start == 1 ) { // otherwise, just started?
            logMessage( LOG_INFO_LEVEL, "case 2 (begin)" );
            bytes_modified = ( SMSA_BLOCK_SIZE - block_start_offset );
            foo += block_start_offset;
            start = 0;
        }
        else if( len < SMSA_BLOCK_SIZE ) { // otherwise, have less than a block's worth of data to go?
            logMessage( LOG_INFO_LEVEL, "case 3 (end)" );
            bytes_modified = block_end_offset;
        }
        else bytes_modified = SMSA_BLOCK_SIZE;
        
        memcpy( foo, buf, bytes_modified );
        
        buf += bytes_modified;
        len -= bytes_modified;
        
        // seek back to the original block
        op = smsa_cmdgen( SMSA_SEEK_BLOCK, null, block );
        ret = smsa_operation( op, null );
        
        if( ret != 0 ) return -1;
        
        // write data
        op = smsa_cmdgen( SMSA_DISK_WRITE, null, null );
        ret = smsa_operation( op, block_data ); // this moves the block pointer forward one block once it's done
        
        if( ret != 0 ) return -1;
        
        // move to next drum and seek to block 0 if at the end of the current drum
        if( block == ( SMSA_BLOCK_SIZE - 1 ) ) {
            drum++;
            op = smsa_cmdgen( SMSA_SEEK_DRUM, drum, null );
            ret = smsa_operation( op, buf );
            
            op = smsa_cmdgen( SMSA_SEEK_BLOCK, null, 0 );
            ret = smsa_operation( op, buf );
        }
        
        // block will overflow to 0 by design when it hits 255
        // block now matches the actual block pointer, which was moved forward when the write was done
        block++;
        
    }
    
    return 0;
    
}

// generates a bitmask of (power) high bits starting from lsb
uint32_t allones( uint8_t power ) {
    return ( pow( 2, power ) - 1 );
}

uint32_t smsa_cmdgen( uint8_t op, uint8_t drum, uint8_t block ) {
    uint32_t out = 0;
    out |= ( op & allones( 6 ) ) << ( 8 + 14 + 4 ); // opcode
    out |= ( drum & allones( 4 ) ) << ( 8 + 14 ); // drum id
    out |= block; // block id
    return out;
}

int smsa_save() {
    int ret = 0;
    
    logMessage( LOG_INFO_LEVEL, "Saving smsa.bin...\n" );
    
    // attempt to open disk array
    disk_array = fopen( "smsa.bin", "w+" );
    
    // deal with errors
    if( disk_array == null ) {
        logMessage( LOG_INFO_LEVEL, "Error opening smsa.bin for saving\n" );
        perror( "The error returned was" );
        exit( -1 );
    }
    
    // initialize temporary storage for array data
    uint8_t *disk_array_data = calloc( MAX_SMSA_VIRTUAL_ADDRESS, 1 );
    
    ret = smsa_vread( 0, MAX_SMSA_VIRTUAL_ADDRESS, disk_array_data );
    
    if( ret != 0 ) return -1;
    
    ret = smsa_operation( smsa_cmdgen( SMSA_UNMOUNT, null, null ), null );
    
    if( ret != 0 ) return -1;
    
    // write data to file
    fseek( disk_array, 0L, SEEK_SET );
    size_t amt = fwrite( disk_array_data, 1, MAX_SMSA_VIRTUAL_ADDRESS, disk_array );
    
    // cleanup
    free( disk_array_data );
    fclose( disk_array );
    
    logMessage( LOG_INFO_LEVEL, "smsa.bin wrote successfully! (bytes written=0x%lX)\n", amt );
    
    return 0;
    
}

int smsa_load() {
    int ret = 0;
    
    logMessage( LOG_INFO_LEVEL, "Opening smsa.bin...\n" );
    
    // try to open the file for reading
    disk_array = fopen( "smsa.bin", "r" );
    
    // if that fails, it might not exist
    // try opening it for reading and writing, creating a new file in the process
    if( disk_array == null ) {
        logMessage( LOG_INFO_LEVEL, "WARNING: smsa.bin does not exist, creating a new file...\n" );
        disk_array = fopen( "smsa.bin", "w+" );
    }
    
    // if that fails, some other error occured
    if( disk_array == null ) {
        logMessage( LOG_INFO_LEVEL, "Error opening smsa.bin\n" );
        perror( "The error returned was" );
        exit( -1 );
    }
    
    // prepare some memory that'll temporarily hold the file's data
    uint8_t *disk_array_data = calloc( MAX_SMSA_VIRTUAL_ADDRESS, 1 );
    
    ret = smsa_operation( smsa_cmdgen( SMSA_MOUNT, null, null ), null );
    
    if( ret != 0 ) return -1;
    
    // determine file size
    fseek( disk_array, 0L, SEEK_END );
    unsigned long loc = ftell( disk_array );
    
    // fill the disk array in memory with the data from the disk array on the hard disk
    if( loc != MAX_SMSA_VIRTUAL_ADDRESS ) { // data is corrupted, start fresh
    
        // this is a little redundant if the file is empty
        if( loc != 0 ) logMessage( LOG_INFO_LEVEL, "WARNING: smsa.bin is not sized properly, reformatting... (size=0x%lX)\n", loc );
        
        ret = smsa_vwrite( 0, MAX_SMSA_VIRTUAL_ADDRESS, disk_array_data );
        
        if( ret != 0 ) return -1;
    }
    else { // data is intact, load that instead
    
        rewind( disk_array );
        fread( disk_array_data, 1, MAX_SMSA_VIRTUAL_ADDRESS, disk_array );
        
        logMessage( LOG_INFO_LEVEL, "disk_array_data[0]=0x%X", disk_array_data[0] );
        
        ret = smsa_vwrite( 0, MAX_SMSA_VIRTUAL_ADDRESS, disk_array_data );
        
        if( ret != 0 ) return -1;
        
        logMessage( LOG_INFO_LEVEL, "smsa.bin opened successfully\n" );
    }
    
    free( disk_array_data );
    
    fclose( disk_array );
    
    return 0;
}