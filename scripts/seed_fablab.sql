-- Seed fablab data for local development/testing
-- Safe to run multiple times (ON CONFLICT / WHERE NOT EXISTS).
-- AI generated

BEGIN;

-- 1) Roles
INSERT INTO roles (name) VALUES
  ('admin'),
  ('maker'),
  ('instructor'),
  ('staff'),
  ('student')
ON CONFLICT (name) DO NOTHING;

-- 2) Users (40 users + 1 admin)
-- Password for all seeded users: "Fablab2026!"
-- Uses bcrypt compatible with backend check (cost 14).
CREATE EXTENSION IF NOT EXISTS pgcrypto;

INSERT INTO users (
  access_key,
  email,
  password,
  first_name,
  last_name,
  tva,
  facturation_address,
  facturation_account,
  activation_code,
  status
) VALUES
  ('fab-admin-001', 'admin@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Camille', 'Durand', 'FR00000000001', '12 Rue des Ateliers, 75011 Paris', 'FR7612345678901234567890123', 'seed-admin-001', 'active'),
  ('fab-user-001', 'lea.martin@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Lea', 'Martin', 'FR00000000002', '3 Rue des Makers, 69001 Lyon', 'FR7612345678901234567890001', 'seed-user-001', 'active'),
  ('fab-user-002', 'hugo.bernard@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Hugo', 'Bernard', 'FR00000000003', '8 Impasse Proto, 33000 Bordeaux', 'FR7612345678901234567890002', 'seed-user-002', 'active'),
  ('fab-user-003', 'chloe.robert@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Chloe', 'Robert', 'FR00000000004', '22 Rue des Plans, 59000 Lille', 'FR7612345678901234567890003', 'seed-user-003', 'active'),
  ('fab-user-004', 'louis.richard@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Louis', 'Richard', 'FR00000000005', '5 Quai Usinage, 44000 Nantes', 'FR7612345678901234567890004', 'seed-user-004', 'active'),
  ('fab-user-005', 'emma.petit@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Emma', 'Petit', 'FR00000000006', '15 Place Num, 13001 Marseille', 'FR7612345678901234567890005', 'seed-user-005', 'active'),
  ('fab-user-006', 'gabriel.moreau@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Gabriel', 'Moreau', 'FR00000000007', '2 Rue du Fraisage, 67000 Strasbourg', 'FR7612345678901234567890006', 'seed-user-006', 'active'),
  ('fab-user-007', 'jade.simon@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Jade', 'Simon', 'FR00000000008', '41 Rue Fab, 35000 Rennes', 'FR7612345678901234567890007', 'seed-user-007', 'active'),
  ('fab-user-008', 'noah.laurent@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Noah', 'Laurent', 'FR00000000009', '9 Avenue Laser, 31000 Toulouse', 'FR7612345678901234567890008', 'seed-user-008', 'active'),
  ('fab-user-009', 'manon.dubois@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Manon', 'Dubois', 'FR00000000010', '17 Rue PLA, 06000 Nice', 'FR7612345678901234567890009', 'seed-user-009', 'active'),
  ('fab-user-010', 'timeo.fontaine@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Timeo', 'Fontaine', 'FR00000000011', '6 Rue Atelier Bois, 21000 Dijon', 'FR7612345678901234567890010', 'seed-user-010', 'active'),
  ('fab-user-011', 'sarah.chevalier@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Sarah', 'Chevalier', 'FR00000000012', '28 Rue CNC, 34000 Montpellier', 'FR7612345678901234567890011', 'seed-user-011', 'active'),
  ('fab-user-012', 'nathan.girard@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Nathan', 'Girard', 'FR00000000013', '11 Rue Design, 37000 Tours', 'FR7612345678901234567890012', 'seed-user-012', 'active'),
  ('fab-user-013', 'lina.perrin@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Lina', 'Perrin', 'FR00000000014', '4 Rue Assemblage, 45000 Orleans', 'FR7612345678901234567890013', 'seed-user-013', 'active'),
  ('fab-user-014', 'ethan.roger@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Ethan', 'Roger', 'FR00000000015', '52 Rue Impression, 80000 Amiens', 'FR7612345678901234567890014', 'seed-user-014', 'active'),
  ('fab-user-015', 'zoe.garnier@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Zoe', 'Garnier', 'FR00000000016', '1 Rue Open Source, 67000 Strasbourg', 'FR7612345678901234567890015', 'seed-user-015', 'active'),
  ('fab-user-016', 'arthur.faure@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Arthur', 'Faure', 'FR00000000017', '14 Rue Prototypage, 25000 Besancon', 'FR7612345678901234567890016', 'seed-user-016', 'active'),
  ('fab-user-017', 'louise.nguyen@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Louise', 'Nguyen', 'FR00000000018', '18 Rue Capteur, 57000 Metz', 'FR7612345678901234567890017', 'seed-user-017', 'active'),
  ('fab-user-018', 'maxime.henry@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Maxime', 'Henry', 'FR00000000019', '7 Rue Robotique, 16000 Angouleme', 'FR7612345678901234567890018', 'seed-user-018', 'active'),
  ('fab-user-019', 'clara.riviere@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Clara', 'Riviere', 'FR00000000020', '10 Rue Lab, 17000 La Rochelle', 'FR7612345678901234567890019', 'seed-user-019', 'active'),
  ('fab-user-020', 'yanis.blanc@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Yanis', 'Blanc', 'FR00000000021', '26 Rue Atelier Metal, 72000 Le Mans', 'FR7612345678901234567890020', 'seed-user-020', 'active'),
  ('fab-user-021', 'ines.marchand@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Ines', 'Marchand', 'FR00000000022', '31 Rue Arduino, 87000 Limoges', 'FR7612345678901234567890021', 'seed-user-021', 'active'),
  ('fab-user-022', 'paul.guerin@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Paul', 'Guerin', 'FR00000000023', '12 Rue Thermoformage, 14000 Caen', 'FR7612345678901234567890022', 'seed-user-022', 'active'),
  ('fab-user-023', 'alice.moulin@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Alice', 'Moulin', 'FR00000000024', '47 Rue PCB, 51100 Reims', 'FR7612345678901234567890023', 'seed-user-023', 'active'),
  ('fab-user-024', 'mael.colin@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Mael', 'Colin', 'FR00000000025', '25 Rue Vecteur, 64000 Pau', 'FR7612345678901234567890024', 'seed-user-024', 'active'),
  ('fab-user-025', 'lucie.noel@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Lucie', 'Noel', 'FR00000000026', '8 Rue Decoupe, 68100 Mulhouse', 'FR7612345678901234567890025', 'seed-user-025', 'active'),
  ('fab-user-026', 'raphael.masson@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Raphael', 'Masson', 'FR00000000027', '20 Rue Atelier Textile, 49000 Angers', 'FR7612345678901234567890026', 'seed-user-026', 'active'),
  ('fab-user-027', 'jade.picard@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Jade', 'Picard', 'FR00000000028', '6 Rue Impression 3D, 38000 Grenoble', 'FR7612345678901234567890027', 'seed-user-027', 'active'),
  ('fab-user-028', 'nolan.lefevre@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Nolan', 'Lefevre', 'FR00000000029', '13 Rue FabAcademy, 63100 Clermont-Ferrand', 'FR7612345678901234567890028', 'seed-user-028', 'active'),
  ('fab-user-029', 'camille.renaud@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Camille', 'Renaud', 'FR00000000030', '34 Rue Open Hardware, 76000 Rouen', 'FR7612345678901234567890029', 'seed-user-029', 'active'),
  ('fab-user-030', 'thomas.lemoine@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Thomas', 'Lemoine', 'FR00000000031', '45 Rue Makerspace, 49100 Angers', 'FR7612345678901234567890030', 'seed-user-030', 'active'),
  ('fab-user-031', 'maelys.pascal@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Maelys', 'Pascal', 'FR00000000032', '19 Rue Fabrique, 26000 Valence', 'FR7612345678901234567890031', 'seed-user-031', 'active'),
  ('fab-user-032', 'leo.clement@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Leo', 'Clement', 'FR00000000033', '7 Rue Gravure, 73000 Chambery', 'FR7612345678901234567890032', 'seed-user-032', 'active'),
  ('fab-user-033', 'jeanne.brunet@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Jeanne', 'Brunet', 'FR00000000034', '29 Rue Atelier Numerique, 64100 Bayonne', 'FR7612345678901234567890033', 'seed-user-033', 'active'),
  ('fab-user-034', 'mathis.lopez@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Mathis', 'Lopez', 'FR00000000035', '2 Rue Usinage CNC, 30000 Nimes', 'FR7612345678901234567890034', 'seed-user-034', 'active'),
  ('fab-user-035', 'eva.lambert@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Eva', 'Lambert', 'FR00000000036', '16 Rue Decoupe Laser, 17100 Saintes', 'FR7612345678901234567890035', 'seed-user-035', 'active'),
  ('fab-user-036', 'sacha.meyer@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Sacha', 'Meyer', 'FR00000000037', '23 Rue Projet Perso, 22000 Saint-Brieuc', 'FR7612345678901234567890036', 'seed-user-036', 'active'),
  ('fab-user-037', 'lola.schmitt@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Lola', 'Schmitt', 'FR00000000038', '11 Rue Formation, 10000 Troyes', 'FR7612345678901234567890037', 'seed-user-037', 'active'),
  ('fab-user-038', 'adam.roy@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Adam', 'Roy', 'FR00000000039', '9 Rue Outillage, 28000 Chartres', 'FR7612345678901234567890038', 'seed-user-038', 'active'),
  ('fab-user-039', 'nina.dupuy@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Nina', 'Dupuy', 'FR00000000040', '40 Rue Atelier Electronique, 41000 Blois', 'FR7612345678901234567890039', 'seed-user-039', 'active'),
  ('fab-user-040', 'malo.gallet@fablab.local', crypt('Fablab2026!', gen_salt('bf', 14)), 'Malo', 'Gallet', 'FR00000000041', '5 Rue Impression PLA, 11000 Carcassonne', 'FR7612345678901234567890040', 'seed-user-040', 'active')
ON CONFLICT (email) DO NOTHING;

-- 3) Role assignments
INSERT INTO users_roles (user_id, role_id)
SELECT u.id, r.id
FROM users u
JOIN roles r ON r.name = 'admin'
WHERE u.email = 'admin@fablab.local'
ON CONFLICT (user_id, role_id) DO NOTHING;

INSERT INTO users_roles (user_id, role_id)
SELECT u.id, r.id
FROM users u
JOIN roles r ON r.name = 'maker'
WHERE u.email LIKE '%@fablab.local'
ON CONFLICT (user_id, role_id) DO NOTHING;

INSERT INTO users_roles (user_id, role_id)
SELECT u.id, r.id
FROM users u
JOIN roles r ON r.name = 'instructor'
WHERE u.email IN (
  'sarah.chevalier@fablab.local',
  'alice.moulin@fablab.local',
  'maelys.pascal@fablab.local',
  'admin@fablab.local'
)
ON CONFLICT (user_id, role_id) DO NOTHING;

INSERT INTO users_roles (user_id, role_id)
SELECT u.id, r.id
FROM users u
JOIN roles r ON r.name = 'staff'
WHERE u.email IN (
  'camille.renaud@fablab.local',
  'thomas.lemoine@fablab.local',
  'admin@fablab.local'
)
ON CONFLICT (user_id, role_id) DO NOTHING;

-- 4) Resources (fablab machines)
INSERT INTO resources (
  uuid, type, zone, name, manual, price_booking_in_eur, price_usage_in_eur, approved
) VALUES
  ('res-cnc-shapeko-01', 'cnc', 'Atelier Bois', 'CNC Shapeoko XXL', 'https://docs.fablab.local/cnc-shapeko', 2.50, 9.00, true),
  ('res-cnc-stepcraft-01', 'cnc', 'Atelier Bois', 'CNC Stepcraft D.840', 'https://docs.fablab.local/cnc-stepcraft', 2.00, 8.00, true),
  ('res-laser-thunder-01', 'laser', 'Zone Decoupe', 'Laser Thunder 100W', 'https://docs.fablab.local/laser-thunder', 3.00, 12.00, true),
  ('res-laser-glowforge-01', 'laser', 'Zone Decoupe', 'Glowforge Pro', 'https://docs.fablab.local/glowforge', 2.50, 10.00, true),
  ('res-prusa-mk4-01', '3d_printer', 'Zone Impression 3D', 'Prusa MK4 #1', 'https://docs.fablab.local/prusa-mk4-1', 1.00, 3.00, true),
  ('res-prusa-mk4-02', '3d_printer', 'Zone Impression 3D', 'Prusa MK4 #2', 'https://docs.fablab.local/prusa-mk4-2', 1.00, 3.00, true),
  ('res-bambu-x1c-01', '3d_printer', 'Zone Impression 3D', 'Bambu X1C', 'https://docs.fablab.local/bambu-x1c', 1.50, 4.00, true),
  ('res-sla-form3-01', 'sla_printer', 'Zone Impression Resine', 'Formlabs Form 3', 'https://docs.fablab.local/form3', 2.00, 6.00, true),
  ('res-vinyl-cameo-01', 'vinyl_cutter', 'Zone Proto Rapide', 'Silhouette Cameo 4', 'https://docs.fablab.local/cameo4', 0.50, 1.50, true),
  ('res-embroidery-01', 'embroidery', 'Textile', 'Brother Innov-is V3', 'https://docs.fablab.local/brother-v3', 1.00, 4.50, true),
  ('res-electronics-01', 'electronics_bench', 'Electro', 'Banc Electronique #1', 'https://docs.fablab.local/electro-bench-1', 0.00, 2.00, true),
  ('res-electronics-02', 'electronics_bench', 'Electro', 'Banc Electronique #2', 'https://docs.fablab.local/electro-bench-2', 0.00, 2.00, true),
  ('res-woodshop-01', 'woodshop_bench', 'Atelier Bois', 'Etabli Bois Principal', 'https://docs.fablab.local/wood-bench', 0.00, 1.00, true),
  ('res-metalsaw-01', 'metal_saw', 'Atelier Metal', 'Scie a Ruban Metal', 'https://docs.fablab.local/metal-saw', 1.50, 5.00, true),
  ('res-welder-01', 'welder', 'Atelier Metal', 'Poste MIG/TIG', 'https://docs.fablab.local/welder', 2.00, 7.50, true)
ON CONFLICT (uuid) DO NOTHING;

-- 5) Sessions/bookings for realism
-- Creates planned bookings across machines for first 20 users.
INSERT INTO sessions (user_id, resource_uuid, started_at, ended_at, time_used, status)
SELECT
  u.id,
  r.uuid,
  NOW() + ((u.id % 10) || ' days')::interval + ((r.id % 6) || ' hours')::interval,
  NOW() + ((u.id % 10) || ' days')::interval + ((r.id % 6 + 2) || ' hours')::interval,
  0,
  'planned'
FROM users u
JOIN resources r ON r.approved = true
WHERE u.email LIKE '%@fablab.local'
  AND u.email <> 'admin@fablab.local'
  AND u.id % 2 = 0
  AND r.id % 3 = 0
  AND NOT EXISTS (
    SELECT 1
    FROM sessions s
    WHERE s.user_id = u.id
      AND s.resource_uuid = r.uuid
      AND s.status = 'planned'
  );

COMMIT;

-- Quick summary output
SELECT 'users' AS table_name, COUNT(*) AS total FROM users
UNION ALL
SELECT 'roles', COUNT(*) FROM roles
UNION ALL
SELECT 'users_roles', COUNT(*) FROM users_roles
UNION ALL
SELECT 'resources', COUNT(*) FROM resources
UNION ALL
SELECT 'sessions', COUNT(*) FROM sessions
ORDER BY table_name;
