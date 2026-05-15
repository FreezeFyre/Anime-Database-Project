# Linux deployment

This deployment setup runs the `main` branch from:

```text
/home/lukeraykovitz/prod/Anime-Database-Project
```

It uses:

- `anime-database.service` to run the backend.
- `anime-database-update.service` to pull, build, test, and restart.
- `anime-database-update.timer` to check for updates every 5 minutes.

## Install on Ubuntu

Install build tools:

```bash
sudo apt update
sudo apt install -y git cmake ninja-build build-essential
```

Create the production clone:

```bash
mkdir -p ~/prod
git clone --branch main git@github.com:FreezeFyre/Anime-Database-Project.git ~/prod/Anime-Database-Project
```

Install the deploy script and systemd units:

```bash
sudo install -m 755 ~/prod/Anime-Database-Project/scripts/deploy-main.sh /usr/local/bin/anime-database-deploy-main
sudo install -m 644 ~/prod/Anime-Database-Project/deploy/systemd/anime-database.service /etc/systemd/system/anime-database.service
sudo install -m 644 ~/prod/Anime-Database-Project/deploy/systemd/anime-database-update.service /etc/systemd/system/anime-database-update.service
sudo install -m 644 ~/prod/Anime-Database-Project/deploy/systemd/anime-database-update.timer /etc/systemd/system/anime-database-update.timer
sudo systemctl daemon-reload
```

Run the first deployment:

```bash
sudo systemctl start anime-database-update.service
```

Start the app and auto-update timer:

```bash
sudo systemctl enable --now anime-database.service
sudo systemctl enable --now anime-database-update.timer
```

## Useful commands

Check status:

```bash
systemctl status anime-database.service
systemctl status anime-database-update.timer
```

View logs:

```bash
journalctl -u anime-database.service -f
journalctl -u anime-database-update.service -f
```

Stop production:

```bash
sudo systemctl stop anime-database.service
```

Disable auto-updates:

```bash
sudo systemctl disable --now anime-database-update.timer
```
