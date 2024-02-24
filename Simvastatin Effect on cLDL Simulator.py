import numpy as np
import math
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

## Time course
number_of_days = 0; 
Time = np.array([0])


"""
Set the parementers of the statin and cLDL model: 
"""

#For cLDL concentration in the blood:
cldl_baseline = 92 #Baseline cLDL concentration in the blood (mg/dl)
kin_cldl = 1.14 * 24  #kim et al recorded a kin of 1.14 per hour, so 1.14 *24 will give the total synthesis of LDL into the blood stream. 
E_max = 0.489
E_cso = 0.0868*pow(10,-4) #mg/dl (this is such a small )
k_out_cldl = kin_cldl/cldl_baseline 
k_out_reduction_factor = 0.4



#For simvastatin acid concentration in the blood: 
Fin_statin = 4.4 *pow(10, -4) #The peak concentration of simvastatin acind in blood is 4.4 ng/ml, so assume that Fin is 4.4 * 10^-4 mg/dl per day. 
#calculate statin decay rate using equation Concentration = Initial * (1/2)^(time/half-life)
half_life_2_hours = 2
half_life_5_hours = 5
k_decay_2 = np.log(2) / half_life_2_hours
k_decay_5 = np.log(2) / half_life_5_hours
statin_acid_decay = (k_decay_2 + k_decay_5) / 2  # Average decay rate
 
"""Set an array to hold the effect array, which models the percentage reduction of cLDL compared to it's pretreatment level"""
percentage_reduction_array = np.array([0])

"""Set the initial conditions for the inital cocentration of statin(mg/dL) and cLDL(mg/dL) in the blood:"""
initial_cldl = 200
cldl_conc = initial_cldl #mg/dl (from kim et al. 2020)
cldl_untreated_conc = initial_cldl#mg/dl (from kim et al. 2020)
simvastatin_acid_conc= 0 #mg/dl

#Array to hold the cLDL and simvastatin concentration: 
cldl_conc_array = np.array([cldl_conc])
cldl_untreated_arr= np.array([cldl_untreated_conc])
cldl_healthy_array= np.array([cldl_baseline])
simvastatin_acid_array = np.array([simvastatin_acid_conc])


#run simuluation

"""The while loop will run whil the cLDL concentration is greater than the baseline cLDL concentraions:
Each iteration of the while loop will calculate the cLDL concentration and simvastatin concentration after 1 day using the rate equations for cLDL and simvastatin.
The cLDL and simvastatin concentrations will then be appended to their respective arrays. 
"""

number_of_hours = 0
days_array = np.array([number_of_days]) #array to hold the number of hours that have passed and reformat it in terms of days. 

while True:
    number_of_hours = number_of_hours + 1; #marks a new hour, where simvastatin has been adminstered.
    
    dsimvastatin_acid_conc = -1 * (statin_acid_decay*simvastatin_acid_conc) #Calculate the change in simvastatin concentration in the blood (mg/dl)
    simvastatin_acid_conc = simvastatin_acid_conc + dsimvastatin_acid_conc  #Calculate the new simvastatin concentraiton in the blood (mg/dl)
    
    #Update array for days; 
    time_in_days = number_of_hours/24
    days_array = np.append(days_array, time_in_days)    
    
    if number_of_hours%24 == 0: #If 24 hours have passed, then the simvastatin is administered again.
        #Calculate the new time values: 
        number_of_days = number_of_days + 1 #marks a new day, where simvastatin has been adminstered.
        Time = np.append(Time, number_of_days) #append the new time value to the time array to show that a new day has passed.
        
        #Calculate the new simvastatin concentration in the blood, accounting for the daily dose of simvastatin acid. 
        simvastatin_acid_conc = simvastatin_acid_conc + Fin_statin 
        
        #Calculate the new cLDL values in the blood, including the treated, diseased untrated, and healthy. 
        dcldl = kin_cldl*(1 -(E_max*simvastatin_acid_conc)/(E_cso + simvastatin_acid_conc)) - k_out_cldl*cldl_conc*k_out_reduction_factor #rate equation for cLDL concentration in the blood (mg/dl)
        dcldl_untreated = kin_cldl - k_out_cldl*cldl_untreated_conc*k_out_reduction_factor #rate equation for
        
        

        #Calculatethe new cLDL concentration in the blood for all three clincal cases. 
        cldl_conc = cldl_conc + dcldl #calculate the new cLDL concentration in the blood (mg/dl)
        cldl_untreated_conc = cldl_untreated_conc + dcldl_untreated
        
        #Calculate the percentage reduction in the cldl_conc, compared to it's orignal level: 
        percentage_reduction = (initial_cldl - cldl_conc)/initial_cldl
        
        #Append the new cLDL concentration to the cLDL array:
        cldl_conc_array = np.append(cldl_conc_array, cldl_conc) #append the new cLDL concentration to the cLDL array.
        cldl_untreated_arr = np.append(cldl_untreated_arr, cldl_untreated_conc)   
        cldl_healthy_array = np.append(cldl_healthy_array, cldl_baseline) 
        percentage_reduction_array = np.append(percentage_reduction_array, percentage_reduction)
        
    
    #Append the new simvastatin concentration to the simvastatin array:
    simvastatin_acid_array = np.append(simvastatin_acid_array, simvastatin_acid_conc)
    
 
    
    if cldl_conc < cldl_baseline or number_of_days == 360:
        break
    

#Plot the results: 


# Plot simvastatin over time:
plt.subplot(3,1,1)
plt.plot(days_array, simvastatin_acid_array, label='Simvastatin Acid')
plt.title("Simvastatin Acid Concentration in the Blood over Time")
plt.xlabel("Time (days)")
plt.ylabel("Simvastatin Acid Concentration (mg/dL)")

ax1 = plt.gca()
ax1.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))

# Plot cLDL concentrations over time:

# Plot for treated cLDL concentration
plt.subplot(3,1,2)
plt.plot(Time, cldl_conc_array, label='Treated cLDL Concentration')

# Plot for untreated cLDL concentration
plt.plot(Time, cldl_untreated_arr, label='Untreated cLDL Concentration', linestyle='--')

# Plot for healthy baseline cLDL concentration
plt.plot(Time, cldl_healthy_array, label='Healthy Baseline cLDL', linestyle=':')

# Titles and labels
plt.title("cLDL Concentration in the Blood over Time")
plt.xlabel("Time (days)")
plt.ylabel("cLDL Concentration (mg/dL)")

plt.legend()

ax2 = plt.gca()
ax2.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))

#Plot the percentage reduction array: 
plt.subplot(3,1,3)
plt.plot(Time, percentage_reduction_array, label='Total Percentage Reduction in cLDL')
plt.title("Total Percentage Reduction in cLDL over Time")
plt.xlabel("Time (days)")
plt.ylabel("Percentage Reduction in cLDL")


ax3 = plt.gca()
ax3.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))

# Display the legend



# Show the plot
plt.tight_layout()
plt.show()



