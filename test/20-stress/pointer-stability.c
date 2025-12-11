#include <mulle-storage/mulle-storage.h>

#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Hefty struct with 256 bytes of data
struct hefty_struct
{
   int     id;
   int     values[64];  // 64 * 4 = 256 bytes
   char    name[32];    // Additional 32 bytes
};


// Structure to track allocated pointers and their content
struct pointer_tracker
{
   struct hefty_struct  *pointer;
   int                  id;
   int                  checksum;
};


int  main( int argc, char *argv[])
{
   struct mulle_storage        store;
   struct pointer_tracker      *trackers;
   struct hefty_struct         *current;
   unsigned int                i;
   unsigned int                j;
   unsigned int                k;
   unsigned int                allocated_count;
   unsigned int                max_allocated;
   unsigned int                iterations;
   int                         checksum;
   int                         verification_passes;
   int                         total_allocations;
   int                         total_frees;

   printf( "Pointer Stability Test: Proving pointers remain stable during growth/shrinkage\n");
   printf( "=============================================================================\n\n");

   max_allocated = 1000;
   iterations    = 2000;

   // Initialize storage for hefty structs
   _mulle_storage_init( &store,
                        sizeof( struct hefty_struct),
                        alignof( struct hefty_struct),
                        16,  // Start small to force growth
                        NULL);

   // Allocate tracker array
   trackers = calloc( max_allocated, sizeof( struct pointer_tracker));
   if( ! trackers)
   {
      printf( "Failed to allocate tracker array\n");
      return( 1);
   }

   allocated_count = 0;
   verification_passes = 0;
   total_allocations = 0;
   total_frees = 0;

   printf( "Test parameters:\n");
   printf( "  Struct size: %zu bytes\n", sizeof( struct hefty_struct));
   printf( "  Max concurrent allocations: %u\n", max_allocated);
   printf( "  Total iterations: %u\n", iterations);
   printf( "  Initial capacity: 16 structs\n\n");

   printf( "Starting test...\n");

   // Use deterministic pseudo-random sequence
   srand( 42);

   for( i = 0; i < iterations; i++)
   {
      // Deterministic allocation/free pattern
      if( allocated_count < max_allocated && (i % 3 == 0 || allocated_count < 50))
      {
         // Allocate new struct
         current = _mulle_storage_malloc( &store);
         if( ! current)
         {
            printf( "Allocation failed at iteration %u\n", i);
            break;
         }

         // Fill with deterministic data
         current->id = (int) i + 1000;
         checksum = 0;
         for( j = 0; j < 64; j++)
         {
            current->values[ j] = (int) (i * 100 + j);
            checksum += current->values[ j];
         }
         snprintf( current->name, sizeof( current->name), "struct_%u", i);

         // Track this pointer
         trackers[ allocated_count].pointer  = current;
         trackers[ allocated_count].id       = current->id;
         trackers[ allocated_count].checksum = checksum;
         allocated_count++;
         total_allocations++;

         if( i % 100 == 0)
            printf( "Allocated struct %d\n", current->id);
      }
      else if( allocated_count > 0 && i % 5 == 0)
      {
         // Free a deterministic struct (always the middle one)
         j = allocated_count / 2;
         current = trackers[ j].pointer;

         // Verify pointer stability before freeing
         checksum = 0;
         for( k = 0; k < 64; k++)
            checksum += current->values[ k];

         if( current->id != trackers[ j].id || checksum != trackers[ j].checksum)
         {
            printf( "ERROR: Data corruption detected!\n");
            return( 1);
         }

         _mulle_storage_free( &store, current);
         total_frees++;

         // Remove from tracker array
         trackers[ j] = trackers[ allocated_count - 1];
         allocated_count--;

         if( i % 100 == 0)
            printf( "Freed struct %d\n", current->id);
      }

      // Periodic verification
      if( i % 200 == 0 && allocated_count > 0)
      {
         verification_passes++;
         printf( "\nVerification pass %d at iteration %u:\n", verification_passes, i);
         printf( "  Active allocations: %u\n", allocated_count);
         printf( "  Storage count: %u\n", _mulle_storage_get_count( &store));

         for( j = 0; j < allocated_count; j++)
         {
            current = trackers[ j].pointer;
            checksum = 0;
            for( k = 0; k < 64; k++)
               checksum += current->values[ k];

            if( current->id != trackers[ j].id || checksum != trackers[ j].checksum)
            {
               printf( "ERROR: Data corruption during verification!\n");
               return( 1);
            }
         }
         printf( "  All %u pointers verified successfully\n", allocated_count);
      }
   }

   printf( "\nTest completed successfully!\n");
   printf( "Final statistics:\n");
   printf( "  Total allocations: %d\n", total_allocations);
   printf( "  Total frees: %d\n", total_frees);
   printf( "  Final active allocations: %u\n", allocated_count);
   printf( "  Storage count: %u\n", _mulle_storage_get_count( &store));
   printf( "  Verification passes: %d\n", verification_passes);
   printf( "  All pointers remained stable throughout growth and shrinkage\n");

   // Clean up remaining allocations
   for( i = 0; i < allocated_count; i++)
      _mulle_storage_free( &store, trackers[ i].pointer);

   _mulle_storage_done( &store);
   free( trackers);

   return( 0);
}