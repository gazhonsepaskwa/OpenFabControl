#!/usr/bin/env python3

# this scipt is entirely ai generated

import argparse
import os
from pathlib import Path
from typing import Iterable, Optional

import psycopg2


TARGET_COLUMNS = {
    "first_name": 255,
    "last_name": 255,
    "tva": 255,
    "facturation_account": 255,
}


def load_dotenv_if_present(repo_root: Path) -> None:
    env_path = repo_root / ".env"
    if not env_path.exists():
        return

    for line in env_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#") or "=" not in stripped:
            continue
        key, value = stripped.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key and key not in os.environ:
            os.environ[key] = value


def connection_candidates() -> Iterable[str]:
    required = [
        "POSTGRES_USER",
        "POSTGRES_PASSWORD",
        "POSTGRES_HOST",
        "POSTGRES_PORT",
        "POSTGRES_DB",
    ]
    missing = [key for key in required if not os.getenv(key)]
    if missing:
        raise SystemExit(f"Missing DB env vars: {', '.join(missing)}")

    user = os.environ["POSTGRES_USER"]
    password = os.environ["POSTGRES_PASSWORD"]
    host = os.environ["POSTGRES_HOST"]
    port = os.environ["POSTGRES_PORT"]
    db_name = os.environ["POSTGRES_DB"]

    hosts = [host]
    if host == "postgres":
        hosts.extend(["localhost", "127.0.0.1"])

    for candidate_host in hosts:
        yield (
            f"postgresql://{user}:{password}@{candidate_host}:{port}/{db_name}"
            "?sslmode=disable"
        )


def connect_db():
    last_error: Optional[Exception] = None
    for dsn in connection_candidates():
        try:
            return psycopg2.connect(dsn)
        except psycopg2.OperationalError as exc:
            last_error = exc
            continue
    raise SystemExit(f"Database connection failed: {last_error}")


def migrate(dry_run: bool) -> None:
    with connect_db() as conn:
        with conn.cursor() as cur:
            for column, size in TARGET_COLUMNS.items():
                statement = (
                    f"ALTER TABLE users ALTER COLUMN {column} TYPE VARCHAR({size})"
                )
                if dry_run:
                    print(f"[DRY-RUN] {statement}")
                else:
                    cur.execute(statement)
                    print(f"[OK] {statement}")

            cur.execute(
                """
                SELECT column_name, character_maximum_length
                FROM information_schema.columns
                WHERE table_name = 'users'
                  AND column_name = ANY(%s)
                ORDER BY column_name
                """,
                (list(TARGET_COLUMNS.keys()),),
            )
            rows = cur.fetchall()

    print("Current users column sizes:")
    for column_name, max_len in rows:
        print(f"- {column_name}: {max_len}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Migrate users column sizes to match database.go."
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print SQL statements without applying changes.",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    load_dotenv_if_present(repo_root)
    migrate(dry_run=args.dry_run)


if __name__ == "__main__":
    main()
