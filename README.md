# 🛡️ dir_sync – Sincronização de Diretórios com Monitoramento em Tempo Real

Este projeto é um utilitário em C que **sincroniza diretórios** recursivamente e monitora **alterações em tempo real**, replicando as mudanças de um diretório de origem (`source`) para um diretório de destino (`target`).

## 🔧 Funcionalidades

- 🧠 Sincronização recursiva de arquivos e pastas
- 🔁 Monitoramento contínuo usando `inotify`
- 📝 Logs coloridos com descrição da ação (criação, modificação, exclusão)
- 🧹 Ignora arquivos temporários (ex: `.swp`, `4913`, `~`, `.tmp`, `.git` etc.)
- 🗂️ Suporte a múltiplas instâncias para diferentes pares de pastas

---

## 🏗️ Compilação

Você precisa de um compilador C (como GCC). Basta rodar:

```bash
make
````

Ou, para limpar e compilar:

```bash
make clean && make
```

---

## ▶️ Execução

```bash
./dir_sync <origem> <destino>
```

Exemplo:

```bash
./dir_sync ~/.config/nvim/ ~/dev/sysconfig/nvim/
```

> O diretório de destino será sincronizado com o de origem e continuará sendo monitorado por novas alterações.

---

## 📜 Ignorando arquivos temporários

Arquivos e padrões ignorados por padrão:

* `4913` (Vim swap temporário)
* `*.swp`, `*.tmp`
* `*~`
* Arquivos iniciados com `.#[...]` (como `.nfsXYZ`)

A lógica está em `fs_utils.c`:

```c
int is_temporary_file(const char *filename) {
    return (
        strcmp(filename, "4913") == 0 ||
        fnmatch("*.swp", filename, 0) == 0 ||
        fnmatch("*~", filename, 0) == 0 ||
        fnmatch("*.tmp", filename, 0) == 0 ||
        fnmatch(".#*", filename, 0) == 0
    );
}
```

---

## 🖥️ Usando com vários diretórios

Você pode criar um script como `dir_sync_all.sh`:

```bash
#!/bin/bash

BIN="$HOME/bin/dir_sync"

$BIN "$HOME/.config/i3/" "$HOME/dev/sysconfig/i3/" &
$BIN "$HOME/.config/nvim/" "$HOME/dev/sysconfig/nvim/" &
$BIN "$HOME/.config/fish/config.fish" "$HOME/dev/sysconfig/fish/config.fish" &
$BIN "$HOME/.config/starship.toml" "$HOME/dev/sysconfig/starship/starship.toml" &
$BIN "/etc/X11/xorg.conf.d/" "$HOME/dev/sysconfig/xorg.conf.d/" &
$BIN "/etc/optimus-manager/" "$HOME/dev/sysconfig/optimus-manager/" &

wait
```

Dê permissão de execução:

```bash
chmod +x dir_sync_all.sh
```

E execute:

```bash
./dir_sync_all.sh
```

---

## ✅ Criar um serviço systemd

Criar um arquivo de unidade em:

```
~/.config/systemd/user/dir_sync.service
```

Conteúdo do arquivo:
 ```
[Unit]
Description=Monitoramento contínuo de arquivos com dir_sync
After=network.target

[Service]
ExecStart=%h/bin/dir_dir_sync_all.sh
Restart=on-failure
Environment=DISPLAY=:0
Environment=XAUTHORITY=%h/.Xauthority

[Install]
WantedBy=default.target
 ```
Recarregue o systemd e habilite o serviço
```
systemctl --user daemon-reexec
systemctl --user daemon-reload
systemctl --user enable --now dir_sync.service
```
Verifique se está rodando
```
systemctl --user status dir_sync.service
```
Logs
```
journalctl --user -u dir_sync.service -f
```
---

## 📁 Estrutura do Projeto

```
dir_sync/
├── main.c
├── config.c/.h
├── watcher.c/.h        # Lida com inotify
├── utils.c/.h          # Lida com a logica do sistema de arquivos
├── Makefile
├── dir_sync_all.sh         # Script opcional para rodar várias instâncias
└── README.md
```

---

## 🧩 Dependências

* Linux
* Biblioteca padrão C (glibc)
* Header `<sys/inotify.h>`

---

## 📜 Licença

MIT. Livre para modificar e usar.
