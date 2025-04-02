# Ouvrir le fichier en mode lecture et créer un fichier de sortie
files = []

for i in range (4, 6) :
    files.append(f"sim_{i}_ones_stats.csv")
    files.append(f"sim_{i}_zeroes_stats.csv")
    files.append(f"sim_{i}_random_stats.csv")


for elem in files :
    with open(elem, 'r') as infile, open('test'+elem, 'w') as outfile:
        # Lire toutes les lignes du fichier
        lines = infile.readlines()
        
        # Parcourir les lignes et ajouter les préfixes requis
        for i, line in enumerate(lines):
            # Retirer le caractère de nouvelle ligne et ajouter le préfixe
            if i == 0:
                new_line = "SNR," + line.strip() + "\n"
            else:
                new_line = f"{i-1}," + line.strip() + "\n"
            
            # Écrire la nouvelle ligne dans le fichier de sortie
            outfile.write(new_line)
