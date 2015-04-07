#!/bin/bash

#SBATCH --account         CI-WATER
#SBATCH --time            7-00:00:00
#SBATCH --job-name        RPlot_ADHydro
#SBATCH --nodes           1
#SBATCH --ntasks-per-node 2

module swap intel gnu/4.8.1
module load R/3.0.2
R CMD BATCH /project/CI-WATER/tools/build/ADHydro/scripts/Plot_TimeSeries_Distributed_from_displaync.R 
