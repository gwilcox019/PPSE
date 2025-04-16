# Projet PPSE
DEBRIE Maëla
WILCOX Grace

# Axe 1 - Speedup the whole chain
## Using threads
We will use mutex (locks) to avoid concurrent access on the frame error and bit error variables. Because we will run multiple threads at once, we might stop above `f_max` frames simulated.


**todo rédiger**
Changer le monitor pour qu'il prenne des verrous en paramètre
Rajouter 2 verrous pour le count fer et le count ber (permet de pipeliner un peu)
On join les threads après la boucle while et on les crée au début, APRES le reset de la variable (comme ca pas besoin de locks pour ca) 
On a besoin de mettre les pointeurs de fonction en global pour les partager - pas un problème vu que c'est les mêmes pour tous les threads et non modifié
On a 6 CPU : on va faire 6 threads

# Axe 2 - Optimize one blockwith SIMD

## Optimize modulator


## Optimize demodulator

## Optimize monitor
We want to speed up the monitor block, by treating 16 elements at a time. We will use SIMD for that.

We begin by computing the number of computations we'll have to do based on the array length (K, multiple of 16).  
Then, for each part of the array:
- We load the original and received messages
- We compare them using the `vceqq` function: if the values are equal, the result vector will contain a 1 ; if the values are different, it will contain a 9
- We want to count the differences: we add 1 to every element of the array. That way we have a 0 when the values were equal and a 1 when they are different
- We count the number of differences on this part of the array: we use `vaddvq_s8`, that sums the array values into a scalar value.
- We add this number, which is the number of bit errors, to the total bit error count.

If we have at least 1 error and we didn't yet add 1 to the frame error count (we use a flag to trace it), we then add 1 to the total number of frame errors.

We can use this variation using the command line, with the option `-c "monitor-neon"`

**Testing**
To test this monitor, we first use both monitors simultaneously: we add another set of variables to count the number of bit/frame errors, and we count the errors on a frame with both functions at the same time. They should produce the same results.

We first had an issue because we forgot to set those new variables to 0 before every SNR, so the values were different. Then, we had an error because we thought that equal values returned 0 in the result array, and different ones would be 1 ; but that was not the case, so we adapted the code.  
After that, we could see that both our monitors produced the same results:
![alt text](monitor_debug.png)

**Performances**
We can see that this new monitor does not affect the performances, meaning it decodes well:
![monitor_perfs](monitor_upgrade_perfs.jpg)

The time taken for the monitor is (most of the time) also reduced, as we can see on this graph:
![monitor_perfs](monitor_upgrade.jpg)
