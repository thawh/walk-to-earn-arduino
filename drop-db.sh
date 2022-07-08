#!/bin/bash

source .env
export PGPASSWORD=$DB_PASSWORD

echo "Dropping database $DB_NAMEE"
psql -h $DB_HOST -U $DB_USER -p $DB_PORT -c "drop database $DB_NAME;"
echo deleting data
rm -rf data
echo "Database deleted"

