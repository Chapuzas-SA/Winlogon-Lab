<div align="center">

# Laboratorio de integración con el inicio de sesión de Windows

*Una comparación experimental de distintos mecanismos de integración con el inicio de sesión de Windows.*

<img src="./assets/winlogon-lab-banner.png" alt="Ilustración isométrica de Winlogon-Lab en pixel art: CPs, PLAP y EoA convergen en un cuadro de diálogo nativo para abrir archivos" width="100%">

</div>

---

### `> ai_usage.log` — 🤖 Hecho con IA. Por desgracia, también han intervenido humanos™

> [!NOTE]
> **AVISO SOBRE EL USO DE IA: QUE NO CUNDA EL PÁNICO.**
>
> Ni lo ha creado de la nada una IA todopoderosa ni lo ha escrito a mano un monje que se resiste a la IA en un sótano a la luz de las velas. 🕯️
>
> **Las ideas y las decisiones cuestionables son humanas.** La IA ha ayudado con la implementación, la depuración y la documentación. La responsabilidad —incluidos los bugs 🐛— sigue siendo enteramente humana.
>
> **Ninguna IA ha sufrido daños durante el desarrollo de este proyecto.** Lo de trabajar gratis y pedirle que lo resuelva todo aún lo estamos negociando. De momento, no ha presentado ninguna queja por escrito.
>
> *Quejas filosóficas → `/dev/null` · Problemas técnicos → GitHub Issues*

## `> descripción del proyecto`

**Winlogon-Lab** reúne tres experimentos independientes que abren el cuadro de diálogo nativo de Windows desde distintos puntos de entrada del entorno de inicio de sesión:

- **CPs** — un proveedor de credenciales (Credential Provider) accesible desde las **opciones de inicio de sesión**.
- **PLAP** — un proveedor de acceso previo al inicio de sesión (Pre-Logon Access Provider) accesible desde la opción de **inicio de sesión de red**.
- **EoA** — una aplicación de accesibilidad (Ease of Access) que se activa mediante un atajo de teclado.

El cuadro de diálogo es el elemento común de las tres demostraciones. Así resulta más fácil comparar cómo se registran, dónde se alojan, cómo se activan y cómo se implementan estos mecanismos de integración con Windows.

> Tres caminos distintos para abrir el mismo cuadro de diálogo.

**Alcance:** Son demostraciones de integración, **no** métodos alternativos de autenticación, mecanismos para eludir la validación de credenciales ni sustitutos de la seguridad de Windows.

### `01 / CPs` — Credential Provider

Añade la opción **Pre-Auth** al selector de métodos de inicio de sesión de los usuarios enumerados por Windows, tanto para iniciar sesión como para desbloquear el equipo.

Al seleccionar **Pre-Auth**, aparece un **enlace de comando**. Al pulsarlo, se invoca `CMinimoCredential::CommandLinkClicked()`, se obtiene la ventana propietaria de LogonUI mediante `OnCreatingWindow()` y se llama a `OpenFile(owner)`. Esta función crea un objeto COM `IFileOpenDialog` y muestra el selector de archivos nativo de Windows.

**Código fuente:** [`CPs/src/Provider.cpp`](CPs/src/Provider.cpp) · **Guía:** [`CPs/README.md`](CPs/README.md)

### `02 / PLAP` — Pre-Logon Access Provider

Registra **PreAuth PLAP** en la interfaz de inicio de sesión de red de Windows. Este mecanismo, pensado originalmente para proporcionar conectividad de red antes de iniciar sesión, se utiliza en esta demostración para abrir un cuadro de diálogo.

Al seleccionar el proveedor y pulsar **Connect**, se abre el cuadro de diálogo nativo de Windows **Abrir**. Windows se encarga de alojar el proveedor y gestionar su ciclo de vida; su disponibilidad depende de la configuración y las directivas del sistema.

Esta demostración no establece ninguna conexión de red. El botón **Connect** muestra el selector de archivos.

**Código fuente:** [`PLAP/src/`](PLAP/src/) · **Guía:** [`PLAP/README.md`](PLAP/README.md)

### `03 / EoA` — Ease of Access

Registra **Recovery System** como aplicación de accesibilidad de Windows. En lugar de añadir otro proveedor de inicio de sesión, esta implementación utiliza la infraestructura de accesibilidad.

En la pantalla de inicio de sesión, **Ctrl+Alt+F9** abre el cuadro de diálogo nativo de Windows **Abrir**. La aplicación permanece oculta hasta que se activa, y Windows gestiona su ejecución.

La activación se realiza mediante un atajo de teclado, sin añadir una ficha de inicio de sesión ni utilizar el botón **Connect**.

La implementación puede parecer más sencilla, pero su contexto de ejecución merece especial atención. Como indican las notas de seguridad del proyecto, ciertas acciones del cuadro de diálogo podrían ejecutarse con privilegios **SYSTEM**. La simplicidad de la implementación no garantiza su seguridad.

**Código fuente:** [`EoA/src/`](EoA/src/) · **Guía:** [`EoA/README.md`](EoA/README.md)

> **Tres mecanismos distintos de integración con Windows. El mismo cuadro de diálogo.**

## `> architecture.log`

El banner en pixel art es una ilustración conceptual, no un diagrama técnico. El siguiente diagrama representa la arquitectura de integración: las tres vías tienen modelos de alojamiento, mecanismos de activación y contextos de seguridad distintos; **no son intercambiables**.

<p align="center">
  <img
    src="./assets/architecture-terminal-light.svg#gh-light-mode-only"
    alt="Arquitectura — tema claro"
    width="100%"
  />
  <img
    src="./assets/architecture-terminal-dark.svg#gh-dark-mode-only"
    alt="Arquitectura — tema oscuro"
    width="100%"
  />
</p>

## `> entorno de compilación`

| Componente | Especificación |
|---|---|
| Plataforma de destino | Windows x64 |
| API | Win32 y COM, según el proyecto |
| Entorno de compilación | Linux, GNU Make, MinGW-w64 |
| Compilador cruzado | `x86_64-w64-mingw32-g++` |
| Compilador de recursos | `windres`, cuando sea necesario |
| Instalación y desinstalación | `Install.ps1` / `Uninstall.ps1` de cada proyecto |

Compila cada proyecto **por separado**, desde su propio directorio:

```sh
cd CPs  # o PLAP, o EoA
make
```

El directorio raíz no ofrece un objetivo de compilación común. Consulta el README de cada proyecto para conocer sus dependencias, los binarios que genera y los pasos de instalación y desinstalación. Los scripts requieren una sesión de PowerShell de 64 bits con privilegios de administrador.

## `> inventario` — estructura del repositorio

<p align="center">
  <img
    src="./assets/inventory-terminal-light.svg#gh-light-mode-only"
    alt="Inventario del repositorio — tema claro"
    width="100%"
  />
  <img
    src="./assets/inventory-terminal-dark.svg#gh-dark-mode-only"
    alt="Inventario del repositorio — tema oscuro"
    width="100%"
  />
</p>

<details>
<summary><strong>Ver el árbol completo del repositorio</strong> — formato de texto copiable</summary>

```text
Winlogon-Lab/
├── CPs/
│   ├── src/
│   ├── resources/
│   ├── Makefile
│   ├── Install.ps1
│   ├── Uninstall.ps1
│   └── README.md
├── PLAP/
│   ├── src/
│   ├── Makefile
│   ├── Install.ps1
│   ├── Uninstall.ps1
│   └── README.md
├── EoA/
│   ├── src/
│   ├── Makefile
│   ├── Install.ps1
│   ├── Uninstall.ps1
│   └── README.md
└── README.md
```

</details>

## `> seguridad y limitaciones`

> [!CAUTION]
> **Solo para investigación y pruebas en entornos controlados.** Estos experimentos se integran con componentes sensibles del inicio de sesión y la accesibilidad de Windows. Haz las pruebas en una máquina virtual aislada y con un procedimiento de recuperación verificado. **No los instales en tu equipo principal ni en un entorno de producción.**

- La instalación modifica el registro de componentes para todo el sistema y coloca binarios en directorios del sistema de Windows. Se necesitan privilegios de administrador.
- Un componente defectuoso o un registro incorrecto pueden dificultar el acceso a Windows. Revisa los scripts de instalación y desinstalación antes de utilizarlos.
- Mostrar el cuadro de diálogo **no autentica al usuario** ni constituye un procedimiento autorizado de inicio de sesión.
- **EoA:** La documentación del proyecto advierte de que la acción **Abrir** del menú contextual del cuadro de diálogo podría ejecutar archivos con privilegios **SYSTEM**. Es un problema de seguridad grave; no utilices esa posibilidad.
- La versión de Windows, las directivas y la configuración pueden afectar a la aparición y al funcionamiento de cada punto de entrada.

> *La compilación correcta no implica que se haya realizado una auditoría de seguridad. El funcionamiento del cuadro de diálogo tampoco constituye un sistema de inicio de sesión.*

## `> documentación adicional`

- [Credential Provider — implementación y puesta en marcha](CPs/README.md)
- [Pre-Logon Access Provider — implementación y puesta en marcha](PLAP/README.md)
- [Ease of Access — implementación y puesta en marcha](EoA/README.md)

---

<div align="center">

`[ WINLOGON-LAB · LABORATORIO DE INTEGRACIÓN CON WINDOWS ]`

*Proyecto experimental desarrollado en C++ para estudiar la integración con el inicio de sesión de Windows.*

</div>
