import os
import sys
import pandas as pd
import matplotlib.pyplot as plt


def read_text_file(path):
    if not os.path.isfile(path):
        print("File not found.\n")
        return
    with open(path, 'r') as f:
        content = f.read()
    print("\n==== rc_properties.txt contents ====\n")
    print(content)
    print("====================================\n")


def plot_csv(path, skiprows=None, value="Value"):
    if not os.path.isfile(path):
        print("File not found.\n")
        return
    try:
        df = pd.read_csv(path, comment='#', header=0, skiprows=skiprows)
    except Exception as e:
        print(f"Error reading CSV: {e}\n")
        return
    if df.empty:
        print("CSV is empty.\n")
        return
    print("\nColumns detected in CSV:", list(df.columns))
    x_candidates = [c for c in df.columns if c.lower() in ["time", "t"]]
    if x_candidates:
        x_col = x_candidates[0]
    else:
        x_col = df.columns[0]
    y_cols = []
    for col in df.columns:
        if col == x_col:
            continue
        try:
            pd.to_numeric(df[col])
            y_cols.append(col)
        except Exception:
            pass
    if not y_cols:
        print("No numeric columns to plot.\n")
        return
    plt.figure()
    for col in y_cols:
        plt.plot(df[x_col], df[col], label=col)
    plt.xlabel(x_col)
    plt.ylabel(value)
    plt.title(os.path.basename(path))
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()


def menu():
    print("\n=== Python File Viewer + Plotter ===")
    print("1. Read 4D array CSV (4DArrays.csv)")
    print("2. Read signal output CSV (Signal_Output.csv)")
    print("3. Read RC output CSV (rc_output.csv)")
    print("4. Display RC properties text file (rc_properties.txt)")
    print("5. Exit")


def main():
    while True:
        menu()
        choice = input("Enter your choice (1-5): ").strip()
        if choice == "1":
            plot_csv("C:/Users/Jorda/projects/helloworld/4dArrays.csv")
        elif choice == "2":
            plot_csv("C:/Users/Jorda/projects/helloworld/Signal_Output.csv", value = "Volts")
        elif choice == "3":
            plot_csv("C:/Users/Jorda/projects/helloworld/rc_output.csv",
                     skiprows=[0], value = "Volts")
        elif choice == "4":
            read_text_file("C:/Users/Jorda/projects/helloworld/rc_properties.txt")
        elif choice == "5":
            print("Exiting.")
            break
        else:
            print("Invalid choice. Please enter 1-5.\n")


if __name__ == "__main__":
    main()
