import logging
import os
import sqlite3

# These are valid characters useful in permissions
GOOD_PERMS_CHARS = frozenset(
    ch for ch in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
)


class PermissionsVisitor:
    def __init__(self, cloak: str, perms: list[str]):
        self.logger = logging.getLogger(__class__.__name__)
        self.cloak = cloak
        self.perms = perms
        self.do_write = False
        self.logger.info(f"{cloak} -> {",".join(perms)} READ-ONLY")

    def perm_get(self, perm: str) -> bool:
        """
        Lazily gets a permission.

        Returns if user is sticky permissioned _ALL or has that permission.
        """
        if "_ALL" in self.perms:
            return True

        all_chs: list[str] = [*perm]
        for ch in all_chs:
            if ch not in GOOD_PERMS_CHARS:
                self.logger.info(f"Getting '{perm}' isn't a valid permission")
                return False

        return perm.lower() in self.perms

    def perm_set(self, perm: str, flag: bool) -> bool:
        """
        Lazily sets a permission.

        Returns if successful.
        """
        if len(perm) > 0 and perm[0] == "_":
            self.logger.info(f"'{perm}' begins with an underscore")
            return False

        all_chs: list[str] = [*perm]
        for ch in all_chs:
            if ch not in GOOD_PERMS_CHARS:
                self.logger.info(f"Setting '{perm}' isn't a valid permission")
                return False

        lperm: str = perm.lower()

        if flag and lperm not in self.perms:
            self.do_write = True
            self.perms.append(lperm)
            self.logger.info(f"{self.cloak} -> '{",".join(self.perms)}' READ-WRITE")
            return True
        elif not flag and lperm in self.perms:
            self.do_write = True
            self.perms.remove(lperm)
            self.logger.info(f"{self.cloak} -> '{",".join(self.perms)}' READ-WRITE")
            return True
        else:
            self.logger.info(f"Redundant perm write '{lperm}' -> {flag}")
            return False


class PermissionsTable:
    def __init__(self, cloak: str):
        """
        Initializes a permissions table for a certain IRC cloak
        """
        self.logger = logging.getLogger(__class__.__name__)
        self.logger.info("Connecting SQLite ptab")
        self.con = sqlite3.connect("/home/chloresolve/tables/ptab.db")

        existance = self.con.execute(
            "select name from sqlite_master where type='table' and name='ptab';"
        ).fetchone()
        if existance is None:
            self.logger.info("ptab is empty, need to initialize it...")
            self.con.execute("create table ptab(cloak unique, perms);")
            self.con.execute(
                "insert into ptab values(?, ?);",
                (
                    os.environ["CHLOROBOT_OWNER"].lower(),
                    "_ALL",
                ),
            )
            self.con.commit()

        clower = cloak.lower()
        raw_perms = self.con.execute(
            "select perms from ptab where cloak = ?;", (clower,)
        ).fetchone()
        if raw_perms is None:
            self.visitor = PermissionsVisitor(clower, [])
        else:
            self.logger.info(raw_perms)
            (perms,) = raw_perms
            self.visitor = PermissionsVisitor(clower, perms.split(","))

    def __enter__(self) -> PermissionsVisitor:
        """
        Returns a visitor class that can have its permissions modified.
        """
        return self.visitor

    def __exit__(self, exc_type, exc_value, traceback):
        if self.visitor.do_write:
            for perm in self.visitor.perms:
                if perm[0] == "_":
                    self.logger.info("Tried to modify a user with sticky permissions!")
                    self.con.close()
                    return

            self.logger.info("Connecting SQLite ptab for writes")

            self.con.execute(
                "insert or replace into ptab(cloak, perms) values(?, ?);",
                (
                    self.visitor.cloak.lower(),
                    ",".join(self.visitor.perms),
                ),
            )
            self.con.commit()
        else:
            self.logger.info("SQLite ptab was open read-only, no need to save")

        self.logger.info("Disconnecting SQLite ptab")
        self.con.close()
