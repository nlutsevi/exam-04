#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

int		ft_strlen(char *str)
{
	int i = 0;

	while (str[i])
		i++;
	return (i);
}

void	ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

void	fatal_error(void)
{
	ft_putstr_fd("error: fatal\n", 2);
	exit(1);
}

void	ft_error(char *str)
{
	ft_putstr_fd("error: ", 2);
	ft_putstr_fd(str, 2);
	ft_putstr_fd("\n", 2);
}

void	ft_exec_error(char *str)
{
	ft_putstr_fd("error: cannot execute ", 2);
	ft_putstr_fd(str, 2);
	ft_putstr_fd("\n", 2);
	exit (1);
}

void	ft_openpipes(int fd[2], int pipes)
{
	if (pipes)
	{
		if (close(fd[READ]) == -1)
			fatal_error();
		if (dup2(fd[WRITE], STDOUT_FILENO) == -1)
			fatal_error();
		if (close(fd[WRITE]) == -1)
			fatal_error();
	}
}

void	ft_closepipes(int fd[2], int pipes)
{
	if (pipes)
	{
		if (dup2(fd[READ], STDIN_FILENO) == -1)
			fatal_error();
		if (close(fd[READ]) == -1)
			fatal_error();
		if (close(fd[WRITE]) == -1)
			fatal_error();
	}
}

int	ft_cd(char **argv)
{
	int		i = 0;

	while (argv[i])
		i++;
	if (i != 2)
	{
		ft_error("cd: bad arguments");
		return (1);
	}
	if (chdir(argv[1]) == -1)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(argv[1], 2);
		ft_putstr_fd("\n", 2);
		return (1);
	}
	return (0);
}

int	ft_execute(char **argv, char **envp, int pipes)
{
	pid_t	pid;
	int		fd[2];

	if (!strcmp(argv[0], "cd"))
		return (ft_cd(argv));
	if (pipes)
		if (pipe(fd) == -1)
			fatal_error();
	pid = fork();
	if (pid == 0)
	{
		ft_openpipes(fd, pipes);
		if (execve(argv[0], argv, envp) == -1)
			ft_exec_error(argv[0]);
	}
	else
		ft_closepipes(fd, pipes);
	return (0);
}

int	ft_command(char **argv, char **envp)
{
	int	i = -1;
	int ret = 0;
	int begin = 0;
	int	pipes = 0;
	int nproc = 0;

	while (argv[++i])
	{
		if (!strcmp(argv[i], "|") || !(argv[i + 1]))
		{
			pipes = 0;
			if (!strcmp(argv[i], "|"))
			{
				pipes = 1;
				argv[i] = NULL;
			}
			ret = ft_execute(argv + begin, envp, pipes);
			begin  = i + 1;
			nproc++;
		}
	}
	while (nproc-- > 0)
		waitpid(-1, 0, 0);
	return (ret);
}

void	ft_restorefd(int backup_stdin)
{
	int	tmp;

	tmp = dup(STDIN_FILENO);
	if (dup2(backup_stdin, STDIN_FILENO) == -1)
		fatal_error();
	if (close(tmp) == -1)
		fatal_error();
}

int main(int argc, char **argv, char **envp)
{
	int i = 0;
	int begin = 1;
	int backup_stdin = dup(STDIN_FILENO);
	int ret;

	(void)argc;
	while(argv[++i])
	{
		if (!strcmp(argv[i], ";") || !argv[i + 1])
		{
			if (!strcmp(argv[i], ";"))
				argv[i] = NULL;
			ret = ft_command(argv + begin, envp);
			begin = i + 1;
		}
		ret = 0;
		ft_restorefd(backup_stdin);
	}
	return (ret);
}