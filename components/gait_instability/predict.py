"""Small gait instability / fall risk prediction helper.

This script is intentionally small and self-contained. It provides two
convenience functions:

- train_and_save(csv_path, model_path): trains a simple LogisticRegression
  model on a CSV file (expects a binary column named 'fall_risk') and saves
  the model with joblib.
- predict_from_dict(model_path, sample_dict): loads a saved model and
  returns a prediction for a single sample represented as a dict of features.

Usage (from repo root):
  python components/gait_instability/predict.py --train
  python components/gait_instability/predict.py --predict '{"acc_x":0.1, "acc_y":-0.2, ...}'

This file is designed to be a small helper; integrate into your feature branch
as needed.
"""
from __future__ import annotations

import argparse
import json
import os
from typing import Dict, Optional, Tuple

import joblib
import numpy as np
import pandas as pd
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, classification_report
from sklearn.model_selection import train_test_split


def load_csv(csv_path: str) -> pd.DataFrame:
    if not os.path.exists(csv_path):
        raise FileNotFoundError(f"CSV file not found: {csv_path}")
    return pd.read_csv(csv_path)


def prepare_xy(df: pd.DataFrame, target: str = "fall_risk") -> Tuple[pd.DataFrame, pd.Series]:
    if target not in df.columns:
        raise KeyError(f"Target column '{target}' not found in CSV")
    # keep numeric features only for this small helper
    numeric = df.select_dtypes(include=[np.number]).copy()
    if target not in numeric.columns:
        # if target is non-numeric, try to coerce
        numeric[target] = pd.to_numeric(df[target], errors="coerce")
    y = numeric.pop(target)
    X = numeric
    if X.shape[1] == 0:
        raise ValueError("No numeric feature columns found in CSV to train on")
    return X, y


def train_and_save(csv_path: str,
                   model_path: str = "models/baseline/gait_model.pkl",
                   test_size: float = 0.2,
                   random_state: int = 42) -> None:
    """Train a logistic regression model and save it to `model_path`.

    Assumptions:
    - CSV at `csv_path` exists and contains a binary column named 'fall_risk'.
    - Uses numeric columns as features.
    """
    df = load_csv(csv_path)
    X, y = prepare_xy(df, target="fall_risk")
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=test_size, random_state=random_state, stratify=y if len(set(y)) > 1 else None
    )

    clf = LogisticRegression(solver="liblinear", max_iter=200)
    clf.fit(X_train, y_train)

    preds = clf.predict(X_test)
    print("Accuracy:", accuracy_score(y_test, preds))
    print(classification_report(y_test, preds))

    # ensure model dir exists
    model_dir = os.path.dirname(model_path)
    if model_dir and not os.path.exists(model_dir):
        os.makedirs(model_dir, exist_ok=True)

    joblib.dump(clf, model_path)
    print(f"Saved model to: {model_path}")


def predict_from_dict(model_path: str, sample: Dict[str, float]) -> Dict[str, object]:
    """Load a saved model and predict on a single sample dict of features.

    Returns a small dict with the predicted class and probability for the
    positive class (if available).
    """
    if not os.path.exists(model_path):
        raise FileNotFoundError(f"Model not found: {model_path}")

    clf = joblib.load(model_path)

    # convert sample to DataFrame with a single row
    row = pd.DataFrame([sample])
    # keep only numeric columns (model was trained on numeric columns)
    row = row.select_dtypes(include=[np.number])
    if row.shape[1] == 0:
        raise ValueError("Provided sample contains no numeric features")

    pred = clf.predict(row)[0]
    result = {"prediction": int(pred)}
    if hasattr(clf, "predict_proba"):
        proba = clf.predict_proba(row)[0]
        # return probability for the predicted class and positive class (index 1 if binary)
        result["probabilities"] = proba.tolist()
        if len(proba) > 1:
            result["positive_prob"] = float(proba[1])

    return result


def _main_train_default_paths() -> Tuple[str, str]:
    csv_default = "data/processed/processed_data.csv"
    model_default = "models/baseline/gait_model.pkl"
    return csv_default, model_default


def main(argv: Optional[list] = None) -> None:
    parser = argparse.ArgumentParser(description="Small gait instability / fall risk helper")
    parser.add_argument("--train", action="store_true", help="Train a model from CSV")
    parser.add_argument("--csv", type=str, default=None, help="Path to CSV file for training")
    parser.add_argument("--model", type=str, default=None, help="Path to save/load the model")
    parser.add_argument("--predict", type=str, default=None, help="JSON string of a single sample to predict")

    args = parser.parse_args(argv)

    csv_path, model_path = _main_train_default_paths()
    if args.csv:
        csv_path = args.csv
    if args.model:
        model_path = args.model

    if args.train:
        print(f"Training using CSV: {csv_path}")
        train_and_save(csv_path, model_path)
        return

    if args.predict:
        try:
            sample = json.loads(args.predict)
        except Exception:
            raise ValueError("--predict argument must be a valid JSON string repr of a dict")
        out = predict_from_dict(model_path, sample)
        print(json.dumps(out, indent=2))
        return

    parser.print_help()


if __name__ == "__main__":
    main()
