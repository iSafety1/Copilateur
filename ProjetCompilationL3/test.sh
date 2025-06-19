#!/bin/bash

# === CONFIGURATION ===
BIN="./bin/tpcc"
TEST_ROOT="./test"
REPORT="./rapport_tests.txt"

# === MAPPAGE DES CATÉGORIES ===
declare -A CATEGORIES=(
    ["good"]="Tests valides"
    ["sem-err"]="Erreurs sémantiques"
    ["syn-err"]="Erreurs syntaxiques"
    ["warn"]="Avertissements"
)

# === INITIALISATION DES COMPTEURS ===
declare -A TOTAL
declare -A OK

echo "=== RAPPORT DE TESTS ===" > "$REPORT"
echo "" >> "$REPORT"

# === TRAITEMENT D'UNE CATÉGORIE DE TESTS ===
run_tests_in_dir() {
    local dir="$1"
    echo ">> ${CATEGORIES[$dir]} ($dir)" >> "$REPORT"
    local count=0
    local success=0

    for file in "$TEST_ROOT/$dir"/*.tpc; do
        [ -e "$file" ] || continue
        filename=$(basename "$file")
        output=$($BIN -t < "$file" 2>&1)
        code=$?

        # Code attendu
        expected=0
        [[ "$dir" == "sem-err" || "$dir" == "syn-err" ]] && expected=1

        if [[ ($code -eq 0 && $expected -eq 0) || ($code -ne 0 && $expected -eq 1) ]]; then
            echo "  $filename" >> "$REPORT"
            success=$((success + 1))
        else
            echo "  $filename (code=$code)" >> "$REPORT"
        fi

        count=$((count + 1))
    done

    TOTAL[$dir]=$count
    OK[$dir]=$success

    echo "Résultat : $success / $count réussis" >> "$REPORT"
    echo "" >> "$REPORT"
}

# === BOUCLE PRINCIPALE SUR LES DOSSIERS DE TESTS ===
for dir in good sem-err syn-err warn; do
    run_tests_in_dir "$dir"
done

# === AFFICHAGE DES SCORES GLOBAUX ===
echo "=== SCORES GLOBAUX ===" >> "$REPORT"
for cat in "${!CATEGORIES[@]}"; do
    total=${TOTAL[$cat]:-0}
    correct=${OK[$cat]:-0}
    if [[ $total -gt 0 ]]; then
        score=$(awk "BEGIN { printf \"%.2f\", ($correct/$total)*100 }")
        echo "${CATEGORIES[$cat]} : $score%" >> "$REPORT"
    else
        echo "${CATEGORIES[$cat]} : Aucun test trouvé" >> "$REPORT"
    fi
done

# === AFFICHAGE TERMINAL ===
cat "$REPORT"