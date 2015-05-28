# Survey of ARM TrustZone applications

This is an unpublished survey done in the scope of a university course in AALTO university.

## Topic

The ARM TrustZone security extensions have since their introduction in
the ARMv6 architecture gained a lot of traction as a low-cost hardware
primitive for secure computing. TrustZone differs from related
technologies, which typically rely on a dedicated security co-processors
added to the SoC, by separating the ARM CPU into two logical processor
modes (referred to as "secure world" and "normal world"), which are
isolated via hardware-based access control features. Typically,
TrustZone technology is leveraged to run a small, security-specialized
portions of code in the secure world mode, whereas the conventional
operating system and applications are run in the normal world mode.

ARM TrustZone is widely leveraged in mobile devices for trusted
computing use cases involving mobile ecosystem stakeholders. In recent
years, there has also been an increased interest in academia for novel
applications of TrustZone technology. The goal of this topic is to
survey current applications of ARM TrustZone technology in both industry
and academia. Ambitious students should also aim to survey the
standardization efforts involved in the domain, and to identify
potential gaps and open research problems in the area.

