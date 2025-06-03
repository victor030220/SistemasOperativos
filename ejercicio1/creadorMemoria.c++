#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define CANT_ELEMENTOS 10
#define NOMBRE_MEMORIA "memoriaDatos"
#define NOMBRE_BANDERA "bandFin"

int main( )
{
    int idMemoria = shm_open( NOMBRE_MEMORIA, O_CREAT | O_RDWR, 0600 );//Devuelve el id de la mem Comp, el segundo parametro es modo crear o toma la que existe
    ftruncate( idMemoria, sizeof(int) * CANT_ELEMENTOS ); //asigno tamaño a la memoria
    int *memoria = (int *) mmap( NULL, sizeof(int) * CANT_ELEMENTOS , PROT_READ | PROT_WRITE, MAP_SHARED, idMemoria, 0 );
    close(idMemoria);
    
    //Inicializamos el vector de la memoria compartida
    for (int i = 0; i < CANT_ELEMENTOS; i++)
    {
        memoria[i] = ( i +1 ) * 10;
    }
    
    
    munmap(memoria, sizeof(int) * CANT_ELEMENTOS);

    int idBand =  shm_open( NOMBRE_BANDERA, O_CREAT | O_RDWR, 0600 );
    ftruncate( idBand, sizeof(bool));
    bool *band = (bool*) mmap( NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED, idBand, 0);
    close(idBand);
    *band = false;
    munmap( band, sizeof(bool));
    return 0;
}
